# Concurrent Test Execution for SM-DP+ Test Suite

## Overview

The `uns-concurrent.sh` script enables parallel execution of SM-DP+ tests to significantly reduce test suite execution time. Each test runs in its own isolated network namespace with dedicated Python SM-DP+ servers.

## Features

- **Parallel Execution**: Run multiple tests concurrently (default: 4 concurrent tests)
- **Isolated Namespaces**: Each test runs in its own network namespace preventing interference
- **Dedicated Servers**: Each test gets its own NIST and BRP Python servers
- **Organized Logging**: All logs are organized by test in timestamped directories
- **Summary Reports**: Automatic generation of test results summary

## Usage

```bash
# Show help
./uns-concurrent.sh -h

# Run all tests with default concurrency (4)
./uns-concurrent.sh -a

# Run all tests with custom concurrency
./uns-concurrent.sh -a -j 8

# Run tests matching a pattern (automatically runs them)
./uns-concurrent.sh -p AuthenticateClient -j 6

# Run specific tests
./uns-concurrent.sh -j 2 -t smdpp_Tests.TC_SM_DP_ES9_AuthenticateClientNIST_01_Nominal smdpp_Tests.TC_SM_DP_ES9_AuthenticateClientNIST_02_ConfirmationCode
```

## Important Notes

- **Option Order**: Options like `-j` must come before `-t`
- **Resource Usage**: Each concurrent test spawns 2 Python servers, so adjust concurrency based on available resources
- **Port Isolation**: Network namespaces allow all tests to use the same ports (8000/8001)

## Log Structure

```
concurrent_run_YYYYMMDD_HHMMSS_PID/
├── summary.txt                    # Overall test results summary
├── test_1/
│   ├── test_name                 # Name of the test
│   ├── status                    # PASSED/FAILED/RUNNING
│   ├── test_output.log          # TTCN-3 test output
│   ├── merged.log               # Formatted TTCN-3 logs
│   ├── pyserver_nist.log        # NIST Python server log
│   ├── pyserver_brp.log         # BRP Python server log
│   └── namespace.log            # Namespace setup/teardown log
├── test_2/
│   └── ...
```

## Performance Benefits

Running tests concurrently can significantly reduce total execution time:

- **Sequential**: ~71 tests × ~10 seconds/test = ~12 minutes
- **Concurrent (j=8)**: ~71 tests ÷ 8 = ~9 batches × ~10 seconds = ~90 seconds

## Viewing Results

```bash
# View summary of latest run
cat smdpp/latest_concurrent_run/summary.txt

# View specific test's merged log (test_1 = first test, test_2 = second test, etc.)
less smdpp/latest_concurrent_run/test_1/merged.log

# View Python server logs for debugging
less smdpp/latest_concurrent_run/test_1/pyserver_nist.log

# Find failed tests
grep FAILED smdpp/latest_concurrent_run/summary.txt
```

## Tips

1. **Optimal Concurrency**: Start with `-j 4` and increase based on system resources
2. **Debugging**: If a test fails, check both `merged.log` and `pyserver_*.log` files
3. **Reruns**: To rerun failed tests, copy their names from the summary and use `-t`
4. **Pattern Matching**: Use `-p` to run related tests together (e.g., all error tests)

## Implementation Details

- Uses `unshare` to create isolated network namespaces
- Each namespace gets its own loopback interface
- Python servers use separate session databases (`sm-dp-sessions-NIST`, `sm-dp-sessions-BRP`)
- Log merging preserves TTCN-3 formatting while filtering noise
- Bash job control manages parallel execution without external dependencies