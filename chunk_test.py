import socket
import time

host = '127.0.0.1'
port = 8080

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((host, port))

# Send Headers
request_header = (
    "POST /upload HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
)
client.send(request_header.encode())

# Chunk 1
client.send(b"5\r\nHello\r\n")
print("Sent chunk 1...")
time.sleep(5) # Simulate network lag

# Chunk 2
client.send(b"6\r\n World\r\n")
print("Sent chunk 2...")
time.sleep(6)

# End Chunk
client.send(b"0\r\n\r\n")
print("Sent end chunk.")

response = client.recv(4096)
print(response.decode())
client.close()
