#!/usr/bin/env python3
"""
Simple script to test server connection
"""

import requests
import sys

def test_connection(url="http://127.0.0.1:8888/health"):
    try:
        print(f"Testing connection to {url}...")
        response = requests.get(url, timeout=3)
        print(f"Status code: {response.status_code}")
        print(f"Response: {response.text}")
        return True
    except requests.exceptions.RequestException as e:
        print(f"Error: {e}")
        return False
        
if __name__ == "__main__":
    url = sys.argv[1] if len(sys.argv) > 1 else "http://127.0.0.1:8888/health"
    test_connection(url)