# MCP Edit File Hanging Issue - Root Cause Analysis

## Stack Trace Analysis

The stack trace shows the same pattern as before - the server is stuck waiting for the next message:
```
Goroutine 6 - fakenet/conn.go:121 - [select] waiting for input
```

## Root Cause

The `edit_file` handler in `/app/mcp-language-server/internal/tools/edit_file.go` has an index out of bounds panic when dealing with empty files or edge cases:

1. **Line 129**: `lines[lastContentLineIdx]` - If the file is empty, `lines` array is empty and accessing index 0 panics
2. **Line 151**: `lines[endIdx]` - After setting `endIdx = len(lines) - 1`, if `lines` is empty, this becomes -1 and panics

## Why This Causes a Hang

1. The panic occurs in the MCP tool handler
2. The panic is caught by the MCP framework (as shown by the error message: "panic recovered in content tool handler")
3. But the JSONRPC response is never sent because the panic interrupts the normal flow
4. The client waits forever for a response that will never come
5. This creates the same protocol deadlock as before

## The Fix

The edit_file.go needs bounds checking before accessing the lines array:

```go
// At line 129:
if lastContentLineIdx >= 0 && lastContentLineIdx < len(lines) {
    pos := protocol.Position{
        Line:      uint32(lastContentLineIdx),
        Character: uint32(len(lines[lastContentLineIdx])),
    }
} else {
    // Handle empty file case
    pos := protocol.Position{Line: 0, Character: 0}
}

// At line 151:
endChar := uint32(0)
if endIdx >= 0 && endIdx < len(lines) {
    endChar = uint32(len(lines[endIdx]))
}
```

## Pattern

This is the same pattern as the previous hang:
1. A handler panics
2. No JSONRPC response is sent
3. Client waits forever
4. Server waits for next request
5. Deadlock

The difference is this panic is in the MCP layer (edit_file tool) rather than the LSP layer (references handler).