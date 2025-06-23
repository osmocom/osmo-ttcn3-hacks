# Parallel Test Execution for SM-DP+ Tests

## Overview
The SM-DP+ test suite contains 67 independent tests. Running them in parallel can significantly reduce execution time from ~67 minutes to ~10-15 minutes.

## Available Scripts

### 1. uns.sh (Original)
- Sequential execution only
- Usage:
  ```bash
  ./uns.sh                    # Run all tests
  ./uns.sh <TEST_NAME>        # Run single test
  ```

### 2. uns-p.sh (Minimal Parallel Extension)
- Adds parallel execution with minimal changes
- Usage:
  ```bash
  ./uns-p.sh                  # Run all tests sequentially
  ./uns-p.sh <TEST_NAME>      # Run single test
  ./uns-p.sh -p <N>           # Run all tests with N workers
  ./uns-p.sh -p <N> <GREP>    # Run matching tests in parallel
  ```

### 3. uns-enhanced.sh (Full-Featured)
- Advanced features including test listing and results summary
- Usage:
  ```bash
  ./uns-enhanced.sh --list                    # List all tests
  ./uns-enhanced.sh --parallel <N>            # Run with N workers
  ./uns-enhanced.sh --parallel <N> <PATTERN>  # Run matching tests
  ```

## How It Works

1. **Server Management**: Python servers (NIST/BRP) start once and are shared by all tests
2. **Test Isolation**: Each test runs in its own network namespace using `unshare`
3. **Log Management**: Each test generates unique log files that are merged after completion
4. **Result Tracking**: Test outcomes are tracked and summarized

## Example Usage

```bash
# Run all tests with 8 parallel workers
./uns-p.sh -p 8

# Run only AuthenticateClient tests with 4 workers
./uns-p.sh -p 4 "AuthenticateClient"

# List available tests
./uns-enhanced.sh --list

# Run with detailed results summary
./uns-enhanced.sh --parallel 8
```

## Performance Considerations

- Optimal worker count: 4-8 (depending on CPU cores)
- Each test takes 5-30 seconds
- Python servers handle concurrent requests well
- Memory usage: ~100MB per test instance

## Troubleshooting

1. **Server startup issues**: Check ports 8000/8001 are free
2. **Permission errors**: Ensure script has execute permissions
3. **Log merging issues**: Check ttcn3_logmerge is in PATH
4. **Test failures**: Individual test logs are preserved for debugging