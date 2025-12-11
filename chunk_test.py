import requests
import os
import time

# --- CONFIGURATION ---
TARGET_URL = 'http://localhost:8080/upload_test'  # Change to your server's URL
FILE_NAME = './www/assets/images/1337.jpeg'
CHUNK_SIZE = 100  # Bytes per chunk (Keep small to test your server's parsing loop)

# 1. Create a dummy file automatically so you don't get FileNotFoundError
def create_test_file():
    if not os.path.exists(FILE_NAME):
        print(f"[*] Creating dummy file: {FILE_NAME}")
        with open(FILE_NAME, 'w') as f:
            # Write enough data to span multiple chunks
            f.write("This is a test of chunked transfer encoding.\n" * 10)
    else:
        print(f"[*] Using existing file: {FILE_NAME}")

# 2. Define a generator function
# In Python 'requests', passing a generator to 'data' AUTOMATICALLY 
# sets the header "Transfer-Encoding: chunked"
def chunked_file_reader(file_path):
    with open(file_path, 'rb') as f:
        while True:
            chunk = f.read(CHUNK_SIZE)
            if not chunk:
                break
            print(f" -> Sending chunk: {len(chunk)} bytes") # Debug print
            yield chunk
            # Optional: Sleep to test if your server handles non-blocking I/O correctly
            # time.sleep(0.1) 

def run_test():
    create_test_file()
    
    print(f"[*] Sending {FILE_NAME} to {TARGET_URL} as chunks...")
    
    try:
        # Note: We do NOT set 'Content-Length'. Requests handles that.
        headers = {'Content-Type': 'text/plain'} 
        
        response = requests.post(
            TARGET_URL, 
            data=chunked_file_reader(FILE_NAME), 
            headers=headers
        )
        
        print("\n--- Server Response ---")
        print(f"Status: {response.status_code}")
        print(f"Body: {response.text}")
        
    except requests.exceptions.ConnectionError:
        print(f"\n[!] Error: Could not connect to {TARGET_URL}")
        print("    Is your C++ webserver running?")
    except Exception as e:
        print(f"\n[!] Unexpected Error: {e}")

if __name__ == "__main__":
    run_test()