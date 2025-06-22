# MCP Edit File Synchronization Bug - Root Cause and Fix

## The Bug

The `ApplyTextEdits` function in `/app/mcp-language-server/internal/utilities/edit.go` modifies files on disk but never notifies the LSP server about the changes. This causes a cache coherency problem:

1. **File is opened in LSP** - LSP caches the content
2. **Edit is applied** - File is modified on disk but LSP doesn't know
3. **Next edit attempt** - Positions are calculated from disk content but applied to stale LSP cache
4. **Index out of bounds** - Because the cached content doesn't match disk content

## The Sequence

```
1. edit_file called
2. client.OpenFile() - LSP caches version 1
3. getRange() reads from disk to calculate positions
4. ApplyTextEdits() modifies file on disk
5. LSP still has version 1 cached!
6. Next edit_file call:
   - getRange() reads NEW content from disk
   - But LSP still has OLD content cached
   - Positions don't match = crash
```

## The Fix

After `ApplyTextEdits` modifies the file, we need to notify the LSP server:

```go
// After writing the file in ApplyTextEdits
if err := osWriteFile(path, []byte(newContent.String()), 0644); err != nil {
    return fmt.Errorf("failed to write file: %w", err)
}

// TODO: Need to call something like:
// client.NotifyFileChanged(ctx, path, newContent.String())

return nil
```

## Why This Happens

The MCP language server tries to be smart by:
- Using LSP for some operations (to get proper language understanding)
- Using direct file I/O for others (for simplicity)

But this creates a split-brain scenario where the LSP's view and the disk's view can diverge.

## Workaround

The current code should either:
1. Always use LSP operations (workspace/applyEdit) instead of direct file I/O
2. Or notify the LSP after direct file modifications
3. Or close and reopen files to force cache refresh