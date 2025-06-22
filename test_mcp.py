#!/usr/bin/env python3
import json
import sys
import socket
import time

def test_mcp_socket():
    # Connect to the Unix socket
    sock_path = "/tmp/mcp-ttcn3-language-server.sock"
    
    try:
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        sock.connect(sock_path)
        print(f"Connected to {sock_path}")
        
        # Send a simple definition request
        request = {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "definition",
                "arguments": {
                    "symbolName": "f_init"
                }
            }
        }
        
        request_str = json.dumps(request) + "\n"
        print(f"Sending: {request_str.strip()}")
        sock.sendall(request_str.encode())
        
        # Try to receive response with timeout
        sock.settimeout(5.0)
        response = b""
        start_time = time.time()
        
        while time.time() - start_time < 5.0:
            try:
                data = sock.recv(4096)
                if not data:
                    break
                response += data
                if b'\n' in response:
                    break
            except socket.timeout:
                print("Socket timeout after 5 seconds")
                break
        
        if response:
            print(f"Response: {response.decode()}")
        else:
            print("No response received")
            
        sock.close()
        
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    test_mcp_socket()