# MCP/NTT Hanging Issue - Root Cause Analysis

## Stack Trace Analysis

The stack trace showed the language server hanging at:
```
Goroutine 41 - /app/ntt/internal/lsp/jsonrpc2/conn.go:168
github.com/nokia/ntt/internal/lsp/jsonrpc2.(*conn).run (0x6f4d90) [chan receive]
```

This indicates the server was waiting for the next JSONRPC message at line 168 in the run() method:
```go
msg, _, err := c.stream.Read(ctx)  // Line 168
```

## Why This Agrees With The Stack Trace

### 1. JSONRPC Protocol Flow
The JSONRPC protocol requires that every request (with an ID) MUST receive a response. The flow is:
- Client sends request with ID
- Server processes request
- Server MUST send response with matching ID
- Both sides continue to next message

### 2. The Actual Bug
In `/app/ntt/internal/lsp/jsonrpc2/conn.go`, the `replier` function had a critical flaw:

```go
func (c *conn) replier(req Request) Replier {
    return func(ctx context.Context, result interface{}, err error) error {
        call, ok := req.(*Call)
        if !ok {
            return nil  // Notifications don't need response
        }
        response, err := NewResponse(call.id, result, err)
        if err != nil {
            return err  // BUG: Returns error but sends NO RESPONSE!
        }
        _, err = c.write(ctx, response)
        return err
    }
}
```

When `NewResponse` failed (e.g., due to unmarshalable data), the replier would:
1. Return an error
2. But NOT send any response to the client

### 3. Why The Stack Trace Makes Sense
The trace shows we're back at line 168 waiting for the next message. This means:
1. The handler completed (we're past the handler call)
2. The server is ready for the next message
3. BUT the client is still waiting for a response to its previous request
4. Since no response was sent, the client will wait forever
5. The client won't send the next request
6. The server waits forever at line 168 for a message that will never come

### 4. Protocol Deadlock
This creates a protocol-level deadlock:
- **Client state**: Waiting for response to request ID X
- **Server state**: Waiting for next request at line 168
- Neither side will progress because they're waiting for different things

### 5. When This Happens
The bug triggers when:
1. A references request is made after file modification
2. The LSP tries to return a large or complex result
3. `json.Marshal` fails on the result (e.g., cyclic references, unsupported types)
4. `NewResponse` returns an error
5. No response is sent, creating the deadlock

### 6. The Fix
The fix ensures we ALWAYS send a response, even on marshal failure:
```go
response, respErr := NewResponse(call.id, result, err)
if respErr != nil {
    // Create an error response that CAN be marshaled
    errResp := &wireError{
        Code:    -32603,  // Internal error
        Message: fmt.Sprintf("Failed to marshal response: %v", respErr),
    }
    response, _ = NewResponse(call.id, nil, errResp)
}
_, writeErr := c.write(ctx, response)  // Always send SOMETHING
```

## Conclusion
The stack trace perfectly matches the bug: it shows the server waiting for the next message after completing a handler, which indicates the handler finished but failed to send a response. This violates the JSONRPC protocol requirement that every request must receive a response, creating a deadlock where the client waits for a response while the server waits for the next request.