# Parallel Test Execution Plan for SM-DP+ Tests

## Current Situation
- 67 test cases that currently run sequentially
- Each test is independent
- Single Python server instance handles all tests
- Log files get overwritten between tests

## Architecture for Parallel Execution

### 1. Master Process
- Starts Python servers (NIST on 8000, BRP on 8001) once
- Manages a pool of worker processes
- Collects and merges logs after completion
- Handles cleanup

### 2. Worker Processes  
- Each runs a single test in isolated namespace
- Uses unique log file names (e.g., `test_<TESTNAME>_<PID>.log`)
- Connects to shared Python servers
- Reports completion status

### 3. Log Management Strategy
```
/app/tt-smdpp/smdpp/
├── pyserver_nist.log          # Shared NIST server log
├── pyserver_brp.log           # Shared BRP server log  
├── parallel_logs/             # Directory for individual test logs
│   ├── test_TC_Foo_12345.log
│   ├── test_TC_Bar_12346.log
│   └── ...
└── merged.log                 # Final merged log
```

## Implementation Approach

### Option 1: New Parallel Script
Create `uns-parallel.sh` that:
- Starts servers once
- Uses GNU parallel or xargs to run tests
- Each test runs: `uns-worker.sh <TEST_NAME>`
- Merges all logs at the end

### Option 2: Extend Existing uns.sh
Add parallel mode to uns.sh:
- `./uns.sh` - Run all tests sequentially (current behavior)
- `./uns.sh <TEST_NAME>` - Run single test (current behavior)  
- `./uns.sh --parallel <N>` - Run all tests with N workers
- `./uns.sh --parallel <N> <TEST_PATTERN>` - Run matching tests

## Key Challenges

1. **Server Port Conflicts**: Need to ensure only one set of servers runs
2. **Log File Naming**: Must avoid conflicts between parallel tests
3. **Process Synchronization**: Wait for all tests before merging logs
4. **Error Handling**: Capture and report failed tests
5. **Resource Management**: Limit concurrent workers to avoid overload

## Recommended Solution

Implement Option 2 with these features:
- Use lockfile to ensure single server instance
- Use test name + PID for unique log files
- Use GNU parallel for work distribution
- Implement proper cleanup on exit
- Add progress reporting

## Benefits
- Reduce test suite runtime from ~67 minutes to ~10-15 minutes (with 8 workers)
- Better resource utilization
- Easier to identify failing tests
- Can still run tests individually