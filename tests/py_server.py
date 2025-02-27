#!/usr/bin/env python3
"""
Super simple Python HTTP server for testing
"""

import http.server
import socketserver
import os

PORT = 8888
Handler = http.server.SimpleHTTPRequestHandler

class TestHandler(Handler):
    def do_GET(self):
        if self.path == '/health':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(b"OK")
        else:
            super().do_GET()

with socketserver.TCPServer(("", PORT), TestHandler) as httpd:
    print(f"Serving at http://localhost:{PORT}")
    print("Try: http://localhost:8888/health")
    httpd.serve_forever()