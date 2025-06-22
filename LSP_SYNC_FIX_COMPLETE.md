# LSP File Synchronization Fix - Complete Solution

## The Problem

When the MCP's `edit_file` operation modified files on disk, the LSP server (ntt) wasn't notified about the changes. This caused a cache coherency problem:

1. **File opened in LSP** → LSP caches content in memory
2. **MCP edits file** → File modified on disk, LSP cache becomes stale
3. **Next operation** → Positions calculated from disk don't match LSP's cached content
4. **Result** → Index out of bounds errors and hangs

## Root Cause Analysis

### MCP Side
- `ApplyTextEdits` in `/app/mcp-language-server/internal/utilities/edit.go` directly modifies files on disk
- No notification sent to LSP server after modification
- LSP server continues using stale cached content

### LSP (ntt) Side
- ntt maintains in-memory file cache in `internal/fs/file.go`
- Updates cache when receiving `textDocument/didChange` notifications
- Cache version incremented on each change
- Without notification, cache remains at old version with old content

## The Fix

Added notification to LSP after file modifications in `/app/mcp-language-server/internal/tools/edit_file.go`:

```go
if err := utilities.ApplyWorkspaceEdit(edit); err != nil {
    return "", fmt.Errorf("failed to apply text edits: %v", err)
}

// Notify the LSP server that the file has changed
if err := client.NotifyChange(ctx, filePath); err != nil {
    // Log the error but don't fail the operation since the edit was already applied
    toolsLogger.Warn("Failed to notify LSP of file change: %v", err)
}
```

## How It Works

1. **MCP opens file** → `client.OpenFile()` sends `textDocument/didOpen`
2. **MCP calculates edits** → Reads current content from disk
3. **MCP applies edits** → Writes new content to disk
4. **MCP notifies LSP** → `client.NotifyChange()` sends `textDocument/didChange`
5. **LSP updates cache** → ntt's `didChange` handler updates in-memory content
6. **Cache synchronized** → LSP and disk now have same content

## Benefits

1. **No more hangs** - LSP always has current file content
2. **Correct positions** - Edit positions match actual file content
3. **Proper LSP workflow** - Follows LSP protocol correctly
4. **Error resilience** - Logs warning if notification fails but doesn't break operation

## Testing

The fix can be tested by:
1. Making an edit through MCP
2. Immediately making another edit
3. Should work without hanging or errors

## Alternative Approaches Considered

1. **Close and reopen files** - Would work but inefficient
2. **Use workspace/applyEdit** - Better but requires more refactoring
3. **Direct LSP integration** - Most correct but complex

The chosen solution is simple, effective, and follows LSP protocol conventions.