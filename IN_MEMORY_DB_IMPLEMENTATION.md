# In-Memory Database Implementation for SM-DP+ Server

## Overview

Added in-memory session storage capability to the Python SM-DP+ server to eliminate database locking issues during concurrent test execution.

## Problem

The original implementation used file-based `shelve` databases which caused concurrency issues:
- File locking prevented multiple server instances from accessing the same database
- Even with separate database files (NIST/BRP suffixes), race conditions could occur
- Tests would fail randomly due to database access conflicts

## Solution

Implemented an optional in-memory storage mode using Python's `shelve.Shelf` with a dictionary backend.

## Implementation Details

### 1. Command Line Argument
Added `-m, --in-memory` flag to `osmo-smdpp.py`:
```python
parser.add_argument("-m", "--in-memory", help="Use in-memory session storage instead of file-based (perfect for testing)",
                    action='store_true', default=False)
```

### 2. Storage Selection Logic
Modified `SmDppHttpServer.__init__` to conditionally use in-memory storage:
```python
if use_in_memory:
    # Use in-memory dictionary with shelve.Shelf interface for testing
    self.rss = shelve.Shelf(dict())
    logger.info("Using in-memory session storage (perfect for concurrent testing)")
else:
    # Use different session database files for BRP and NIST
    session_db_suffix = "BRP" if use_brainpool else "NIST"
    self.rss = rsp.RspSessionStore(os.path.join(DATA_DIR, f"sm-dp-sessions-{session_db_suffix}"))
    logger.info(f"Using file-based session storage: sm-dp-sessions-{session_db_suffix}")
```

### 3. Concurrent Test Runner Integration
Updated `uns-concurrent.sh` to use the `--in-memory` flag:
```bash
# Start NIST server with in-memory storage
python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8000 --in-memory

# Start BRP server with in-memory storage  
python3 -u ./osmo-smdpp.py -H 127.0.0.1 -p 8001 --brainpool --in-memory
```

## Benefits

1. **No File Locking**: Each server instance has its own in-memory storage
2. **Better Performance**: No disk I/O for session operations
3. **Clean Test Isolation**: Each test run starts with empty session storage
4. **No Cleanup Required**: Storage is automatically cleared when server stops

## Testing

Successfully tested with concurrent execution of 4 tests - all passed without database conflicts.

## Notes

- The logging message only appears with `--verbose` flag (INFO level logging)
- Sessions are not persistent between server restarts (perfect for testing)
- The same `shelve.Shelf` interface is maintained for compatibility