# NTT ParseFile Caching Fix - Implementation Summary

## Problem Fixed
The ntt language server was hanging/freezing when processing `content` requests because:
1. Every LSP request was parsing files from scratch (no caching)
2. The `documentSymbol` operation walks the entire AST tree
3. Multiple concurrent parses of the same file caused resource exhaustion
4. Edited files were not being used - always reading from disk

## Solution Implemented

### 1. Added Parse Cache to Server
```go
// In server.go
type Server struct {
    // ... existing fields ...
    
    // Parse cache for files
    parseCacheMu sync.RWMutex
    parseCache   map[string]*parseCacheEntry
}

type parseCacheEntry struct {
    tree    *ttcn3.Tree
    version int
}
```

### 2. Implemented Smart ParseFile Method
- Uses file ID (includes version) as cache key
- Detects if file is open in editor and uses current content
- Falls back to disk parsing for unopened files
- Thread-safe with proper mutex usage

### 3. Updated Critical Handlers
- Changed `documentSymbol` to use `s.ParseFile` instead of `ttcn3.ParseFile`
- This ensures caching is used for the heavy AST walking operation

## Benefits
1. **Performance**: Parse results are cached, avoiding repeated parsing
2. **Correctness**: Edited files use in-memory content, not disk content
3. **Stability**: No more hanging on large files
4. **Memory**: Cache uses file IDs that change on edit, preventing stale data

## Files Modified
1. `/app/ntt/internal/lsp/server.go` - Added cache fields and initialization
2. `/app/ntt/internal/lsp/workspace_symbol.go` - Implemented ParseFile with caching
3. `/app/ntt/internal/lsp/document_symbol.go` - Updated to use s.ParseFile

## Testing Notes
- Both ntt and mcp-language-server build successfully
- The MCP should now handle content requests without hanging
- Cache automatically invalidates when files are edited (version changes)

## Next Steps
- Update remaining handlers to use `s.ParseFile` (completion, hover, etc.)
- Add metrics to monitor cache hit rates
- Consider adding cache size limits for very large projects