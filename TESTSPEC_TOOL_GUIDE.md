# Test Specification Tool Usage Guide

This guide explains how to effectively use the `/app/tools/structured_md_tool.py` to query and understand test cases from the SGP.23 test specification.

## Tool Location and Basic Usage

```bash
python3 /app/tools/structured_md_tool.py /app/testspec.md --command <command> [options]
```

## Available Commands

### 1. `search` - Find specific test cases or content
```bash
# Search for specific test case by number
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "4.3.14"

# Search for error codes
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "8.2.1 6.7"

# Search for test sequences
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Test Sequence #14"

# Add context lines around matches
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "4.3.14" --context 50
```

### 2. `testcases` - List all test cases
```bash
python3 /app/tools/structured_md_tool.py /app/testspec.md --command testcases
```

### 3. `sections` - Show document structure
```bash
python3 /app/tools/structured_md_tool.py /app/testspec.md --command sections
```

## Understanding Test Case Structure

### Test Case Naming Convention
- Format: `TC_Component_Interface.OperationVariant`
- Example: `TC_SM-DP+_ES9+.AuthenticateClientNIST`
  - Component: SM-DP+
  - Interface: ES9+
  - Operation: AuthenticateClient
  - Variant: NIST

### Test Sequence Structure
Each test case contains:
1. **Initial Conditions** - Setup requirements
2. **Test Steps** - Sequence of operations
3. **Expected Results** - What should happen
4. **Error Codes** - Format: "Subject Code X.X.X Reason Code Y.Y"

## Search Strategies

### Finding Specific Test Cases

1. **By Test Number**:
   ```bash
   # Direct section search
   python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "4.3.14.2.2"
   ```

2. **By Test Name**:
   ```bash
   # Search for specific test implementation
   python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "TC_SM-DP+_ES9+.AuthenticateClientNIST_ErrorCases"
   ```

3. **By Error Scenario**:
   ```bash
   # Search for specific error test
   python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Un-matched EID"
   ```

### Finding Error Codes

When a test expects specific error codes:
```bash
# Search for error code format (subject.reason)
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "8.2.6 3.8"
```

### Finding Test Sequences

For detailed test steps:
```bash
# Find specific test sequence with context
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Test Sequence #01 Nominal: Retry with same otPK" --context 50
```

## Important Notes on Searching

### Escaping Special Characters
The testspec.md contains markdown formatting, so some searches require special handling:

1. **Underscores**: NOT needed to escape in this tool (unlike grep)
   ```bash
   # This works fine
   python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "TC_SM_DP"
   ```

2. **Dots in Section Numbers**: NOT needed to escape
   ```bash
   # This works fine
   python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "4.3.14"
   ```

## Practical Examples

### Example 1: Understanding an Error Test Case
```bash
# Step 1: Find the test case section
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "4.3.14.2.2" --context 20

# Step 2: Find the specific test sequence
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Test Sequence #14" --context 50

# Step 3: Look for the expected error codes
# The test will show format like: MTD_HTTP_RESP( #R_ERROR_8_2_1_6_7)
```

### Example 2: Understanding Retry Scenarios
```bash
# Find retry test cases
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "RetryCases"

# Find specific retry behavior
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Retry with same otPK" --context 100
```

## Quick Reference for Common Searches

```bash
# Find all error test cases for a component
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "ErrorCases"

# Find all AuthenticateClient tests
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "AuthenticateClient"

# Find tests with confirmation codes
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Confirmation Code"

# Find retry scenarios
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "retry"
```

## Cross-Reference Files

- `/app/filtered.md` - Contains line numbers for quick test case lookup
- `/app/tt-smdpp/smdpp/TEST_CASE_ANALYSIS.md` - Detailed analysis with line references
- `/app/sgp22_2.5.md` - Full SGP.22 specification (use selectively, very large)

## Tips

1. Always use `--context` parameter to see surrounding content
2. Start broad, then narrow down (e.g., search section first, then specific test)
3. Look for test sequence numbers to understand the exact scenario
4. Pay attention to Initial Conditions - they define the test setup
5. Error codes follow pattern: "Subject Code X.X.X Reason Code Y.Y"