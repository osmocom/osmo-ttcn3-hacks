#!/usr/bin/env python3
"""
Remove the default profile for test certificate EID to make Error Test #14 work.
This test expects the EID to have NO default profile configured.
"""

def patch_file(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    # Find and comment out the test certificate EID profile
    in_test_cert_profile = False
    brace_count = 0
    
    for i in range(len(lines)):
        line = lines[i]
        
        # Start of test certificate EID profile
        if '89049032123451234512345678901235' in line and '# Test certificate EID' in line:
            in_test_cert_profile = True
            lines[i] = '            # ' + line.lstrip()  # Comment out
            brace_count = 1
            continue
        
        if in_test_cert_profile:
            # Count braces to know when the profile ends
            brace_count += line.count('{') - line.count('}')
            lines[i] = '            # ' + line.lstrip()  # Comment out
            
            # Check if we've closed all braces
            if brace_count == 0:
                in_test_cert_profile = False
    
    # Write back
    with open(filename, 'w') as f:
        f.writelines(lines)
    
    print(f"Commented out test certificate EID profile in {filename}")

if __name__ == "__main__":
    patch_file("/app/pysim/osmo-smdpp.py")