# MCP Debug Setup for Claude

When MCP is running as part of Claude's integrated setup, you can't directly see stderr output. Here's how to enable and capture debug logging:

## Option 1: Add Debug Environment Variables to Claude Config

Edit the MCP server configuration to add logging environment variables:

```json
"ttcn3-language-server": {
  "command": "/app/mcp-language-server/mcp-language-server",
  "args": [
    "--workspace",
    "/app/tt-smdpp/smdpp",
    "--lsp",
    "/app/ntt/bin/ntt",
    "--",
    "langserver"
  ],
  "env": {
    "PATH": "/usr/bin:/bin:/usr/local/bin",
    "LOG_LEVEL": "DEBUG",
    "LOG_TOOLS": "DEBUG",
    "LOG_LSP": "DEBUG"
  }
}
```

## Option 2: Create a Wrapper Script for Logging

Create a wrapper script that captures stderr to a file:

```bash
#!/bin/bash
# /app/mcp-language-server/mcp-wrapper.sh

LOG_FILE="/tmp/mcp-ttcn3-$(date +%Y%m%d-%H%M%S).log"
export LOG_LEVEL=DEBUG
export LOG_TOOLS=DEBUG

exec /app/mcp-language-server/mcp-language-server "$@" 2>"$LOG_FILE"
```

Then update Claude config to use the wrapper:
```json
"command": "/app/mcp-language-server/mcp-wrapper.sh"
```

## Option 3: Modify MCP to Log to File

Add file logging to the MCP server initialization:

```go
// In main.go or similar
logFile, _ := os.OpenFile("/tmp/mcp-debug.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
logging.Writer = io.MultiWriter(os.Stderr, logFile)
```

## Viewing Logs

1. **In Claude**: Use `/mcp` command to see server status
2. **From terminal**: Check log files in `/tmp/mcp-*.log`
3. **Real-time monitoring**: `tail -f /tmp/mcp-debug.log`

## Troubleshooting Stuck MCP

When MCP appears stuck:
1. Check if process is still running: `ps aux | grep mcp-language-server`
2. Look for timeout logs in debug output
3. Check CPU usage to see if it's actually processing
4. Restart the MCP server through Claude's interface

## Key Debug Points Added

The recent changes added debug logging at:
- `workspace/symbol` query start/end
- Search path resolution
- TTCN3 file detection
- Timeout handling (30 seconds for references)
- Result limiting (100 max for workspace/symbol)