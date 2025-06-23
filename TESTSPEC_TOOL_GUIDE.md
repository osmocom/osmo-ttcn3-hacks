# Test Specification Tool Usage Guide

This guide explains how to effectively use the `/app/tools/structured_md_tool.py` to query and understand test cases from the SGP.23 test specification.

## Tool Location and Basic Usage

### Enhanced Tool (RECOMMENDED)
```bash
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md --command <command> [options]
```

### Original Tool (for simple searches)
```bash
python3 /app/tools/structured_md_tool.py /app/testspec.md --command <command> [options]
```

## Available Commands

### Enhanced Tool Commands (RECOMMENDED)

#### 1. `extract-range` - Extract content between patterns (MOST USEFUL)
```bash
# Get all AuthenticateClientNIST error test sequences
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command extract-range \
  --start-pattern "Test Sequence #1 Error: Invalid EUM Certificate" \
  --end-pattern "Test Sequence #21" \
  --format table

# Get GetBoundProfilePackage error cases
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command extract-range \
  --start-pattern "Test Sequence #01 Error: Invalid eUICC Signature" \
  --end-pattern "Test Sequence #06 VOID" \
  --format summary
```

#### 2. `search` - Enhanced search with multi-pattern and regex support
```bash
# Multi-pattern search (comma-separated)
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search \
  --query "Invalid EUM Certificate,Invalid eUICC Certificate,Profile not released" \
  --multi-pattern --format table

# Regex search for complex patterns
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search \
  --query "Test Sequence #\\d+ Error:.*Certificate" \
  --regex --format consolidated

# Simple search with context
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search \
  --query "4.3.14" \
  --context 20
```

#### 3. `extract-section` - Extract complete sections
```bash
# Extract AuthenticateClientNIST ErrorCases section
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command extract-section \
  --section "4.3.14.2.2" \
  --format text

# Extract InitiateAuthentication section
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command extract-section \
  --section "4.3.12" \
  --format summary
```

#### 4. `search-error-codes` - Find SGP.23 error codes
```bash
# Find all error codes in AuthenticateClientNIST
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search-error-codes \
  --section "4.3.14.2.2" \
  --format table

# Find all certificate-related error codes
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search-error-codes \
  --format table | grep -i certificate
```

#### 5. `sections` - Show document structure
```bash
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command sections \
  --format text
```

### Original Tool Commands (for simple searches)

#### 1. `search` - Find specific test cases or content
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

## Output Formats (Enhanced Tool)

The enhanced tool supports multiple output formats for different use cases:

- `--format text` - Full context with surrounding lines (default, most detailed)
- `--format table` - Clean tabular view (best for error codes and comparisons)
- `--format summary` - Just line numbers and matches (concise overview)
- `--format consolidated` - Groups results by section context (best for understanding relationships)
- `--format json` - Structured data for scripting and automation

## When to Use Enhanced vs Original Tool

### Use Enhanced Tool When:
- **Analyzing error test cases** - Range extraction gets all sequences in one command
- **Looking for multiple related patterns** - Multi-pattern search is much faster
- **Need structured output** - Table and consolidated formats are clearer
- **Complex pattern matching** - Regex support handles advanced searches
- **Section analysis** - Extract complete sections with proper boundaries
- **Error code analysis** - Specialized SGP.23 error code detection

### Use Original Tool When:
- **Simple single-pattern searches** - Basic search needs
- **Exploring test case structure** - testcases/operations/procedures commands
- **Learning the document** - Initial exploration and understanding

## Key Advantages of Enhanced Tool

**Original Task**: Analyze all AuthenticateClientNIST error test cases
- **Old way**: 5+ separate search commands, risk of missing sequences
- **New way**: 1 range extraction command gets everything

```bash
# OLD WAY (multiple commands, risk of missing sequences)
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Test Sequence #1 Error" --context 150
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Test Sequence #12 Error" --context 100
# ... many more commands

# NEW WAY (single command, complete coverage)
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command extract-range \
  --start-pattern "Test Sequence #1 Error: Invalid EUM Certificate" \
  --end-pattern "Test Sequence #21" \
  --format table
```

## Practical Examples

### Example 1: Analyzing All AuthenticateClientNIST Error Cases (ENHANCED)
```bash
# Step 1: Get complete error test section in one command
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command extract-section \
  --section "4.3.14.2.2" \
  --format summary

# Step 2: Get all error test sequences with clear table view
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command extract-range \
  --start-pattern "Test Sequence #1 Error: Invalid EUM Certificate" \
  --end-pattern "Test Sequence #21" \
  --format table

# Step 3: Get all error codes in tabular format
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search-error-codes \
  --section "4.3.14.2.2" \
  --format table
```

### Example 2: Finding Certificate-Related Errors Across Document
```bash
# Multi-pattern search for certificate errors
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search \
  --query "Invalid EUM Certificate,Invalid eUICC Certificate,Expired.*Certificate" \
  --multi-pattern --format consolidated

# Regex search for all certificate error patterns
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search \
  --query "Test Sequence #\\d+ Error:.*Certificate" \
  --regex --format table
```

### Example 3: Understanding Retry Scenarios (ENHANCED)
```bash
# Find all retry test cases with range extraction
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command search \
  --query "RetryCases" \
  --format summary

# Get specific retry section completely
python3 /app/tools/enhanced_structured_md_tool.py /app/testspec.md \
  --command extract-section \
  --section "4.3.13.2.4" \
  --format text
```

### Example 4: Legacy Approach with Original Tool
```bash
# Step 1: Find the test case section
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "4.3.14.2.2" --context 20

# Step 2: Find the specific test sequence
python3 /app/tools/structured_md_tool.py /app/testspec.md --command search --query "Test Sequence #14" --context 50

# Step 3: Look for the expected error codes
# The test will show format like: MTD_HTTP_RESP( #R_ERROR_8_2_1_6_7)
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