import sys

# Read input
line = sys.stdin.read().strip()

# Process
processed = line.upper()

# CGI response (header + body)
# print("Content-Type: text/plain\n")
print(processed)
