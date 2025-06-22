# NTT ParseFile Caching Fix Plan

## Root Cause Analysis

The ntt language server has a sophisticated caching infrastructure but **handlers are bypassing it** by calling `ttcn3.ParseFile` directly instead of using the cached file system.

### Current Issues:
1. **Direct ParseFile calls**: 30+ places call `ttcn3.ParseFile(file)` directly
2. **No cache reuse**: Every request parses files from scratch
3. **Heavy operations**: `documentSymbol` walks entire AST for every request
4. **Race conditions**: Concurrent parsing without proper synchronization
5. **Resource exhaustion**: Multiple concurrent parses of large files

### Existing Infrastructure (NOT BEING USED):
- Memoization cache with weak references
- File versioning and automatic cache invalidation
- parseLimit semaphore for concurrency control
- File system abstraction with caching

## Implementation Plan

### Phase 1: Implement Proper Server.ParseFile
```go
func (s *Server) ParseFile(uri string) *ttcn3.Tree {
    // Convert URI to fs.File
    file := s.getOrCreateFile(uri)
    if file == nil {
        return ttcn3.ParseFile(uri) // fallback
    }
    
    // Use cached parse result from file system
    return file.ParseTree() // This uses memoization
}
```

### Phase 2: Update All Handlers
Replace all instances of:
```go
tree := ttcn3.ParseFile(file)
```
With:
```go
tree := s.ParseFile(file)
```

### Phase 3: Add File System Integration
1. Ensure all files are registered in the fs.Store
2. Use fs.File handles for all parsing
3. Leverage existing memoization

### Phase 4: Fix documentSymbol Performance
1. Cache documentSymbol results separately
2. Only recompute when file version changes
3. Add timeout protection

## Files to Modify

### Priority 1 (Core Handlers):
- `/app/ntt/internal/lsp/document_symbol.go` - documentSymbol handler
- `/app/ntt/internal/lsp/hover.go` - hover handler  
- `/app/ntt/internal/lsp/definition.go` - definition handler
- `/app/ntt/internal/lsp/references.go` - references handler
- `/app/ntt/internal/lsp/completion.go` - completion handler (14 instances!)

### Priority 2 (Server Infrastructure):
- `/app/ntt/internal/lsp/server.go` - implement proper ParseFile
- `/app/ntt/internal/lsp/workspace_symbol.go` - fix ParseFile method

### Priority 3 (Other Handlers):
- `/app/ntt/internal/lsp/diagnostics.go`
- `/app/ntt/internal/lsp/formatter.go`
- `/app/ntt/internal/lsp/inlay_hint.go`
- `/app/ntt/internal/lsp/semantic_tokens.go`
- `/app/ntt/internal/lsp/code_lens.go`

## Expected Benefits

1. **Performance**: 10-100x faster for repeated operations
2. **Stability**: No more hanging on large files
3. **Memory**: Reduced memory usage from parse reuse
4. **Concurrency**: Proper synchronization via existing infrastructure

## Testing Strategy

1. Test with large TTCN-3 files (like smdpp_Tests.ttcn)
2. Concurrent requests to same file
3. Verify cache hits/misses
4. Monitor memory usage