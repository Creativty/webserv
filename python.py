#!/usr/bin/env python3
import os
import sys
import time

# --- Required CGI headers ---
time.sleep(5)
print("HTTP/1.0 200 OK\r\n")
# print("Content-Type: text/html\r\n")

# --- Example: read request data ---
method = os.environ.get("REQUEST_METHOD", "GET")

if method == "GET":
    query = os.environ.get("QUERY_STRING", "")
    body = f"<p>Query string: {query}</p>"
elif method == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    data = sys.stdin.read(length) if length > 0 else ""
    body = f"<p>POST data: {data}</p>"
else:
    body = "<p>Unsupported method</p>"

# --- Response body ---
print(f"""
<html>
  <head><title>CGI Example</title></head>
  <body>
    <h1>Hello from Python CGI!</h1>
    <p>Method: {method}</p>
    {body}
  </body>
</html>
""")
