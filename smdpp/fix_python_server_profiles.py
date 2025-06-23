#!/usr/bin/env python3
"""
Fix the Python SM-DP+ server profile configuration.
The event_based_profiles dictionary is being overwritten.
"""

import re

def fix_profiles(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    # Find the duplicate event_based_profiles initialization
    # The second one starts around line 614
    pattern = r'(print\(f"INIT: AC_NO_ELIGIBLE present:.*?\)\s*\n\s*# Event-based SM-DS profiles\s*\n\s*self\.event_based_profiles = \{)'
    
    # Replace the second initialization with an update instead
    def replace_second_init(match):
        return match.group(1).replace('self.event_based_profiles = {', '# Update event_based_profiles with additional profiles\n        self.event_based_profiles.update({')
    
    content = re.sub(pattern, replace_second_init, content, flags=re.DOTALL)
    
    # Also need to fix the closing brace - find the pattern and add a closing parenthesis
    # Find where the second event_based_profiles dict ends
    pattern2 = r'(\'EVENT_RESTRICTED\':[^}]+\}\s*\})'
    
    def fix_closing(match):
        return match.group(0) + ')'
    
    content = re.sub(pattern2, fix_closing, content)
    
    # Save the fixed file
    with open(filename, 'w') as f:
        f.write(content)
    
    print(f"Fixed {filename} - merged event_based_profiles dictionaries")

if __name__ == "__main__":
    fix_profiles("/app/pysim/osmo-smdpp.py")