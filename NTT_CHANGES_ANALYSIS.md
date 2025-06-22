# Analysis of ntt Changes

## Changes Made

1. **ttcn3/db.go** - Added GetUses/TryGetUses with mutex protection
2. **references.go** - Added timeout and panic recovery  
3. **jsonrpc2/conn.go** - Fixed response marshaling to always send response
4. **server.go/tree.go** - Added ParseFile wrapper with recovery

## Were They Necessary?

**No** - The root cause was in the MCP layer not sending `textDocument/didChange` notifications.

## What Should Be Kept?

Only the **jsonrpc2/conn.go** fix is truly valuable:
- Prevents protocol deadlock when response marshaling fails
- Ensures client always gets a response (even if it's an error)
- This is a legitimate bug that could cause hangs in other scenarios

## What Was The Real Fix?

In `/app/mcp-language-server/internal/tools/edit_file.go`:
```go
// Notify the LSP server that the file has changed
if err := client.NotifyChange(ctx, filePath); err != nil {
    toolsLogger.Warn("Failed to notify LSP of file change: %v", err)
}
```

This single change fixed the root cause by ensuring LSP cache stays synchronized with disk.