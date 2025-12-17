import requests
import os
import time

# --- CONFIGURATION ---
TARGET_URL = 'http://localhost:8080/upload'
TARGET_FILE = 'Video.mp4'
FILE_NAME = '/home/aindjare/Downloads/RDT_20251203_083733.mp4'
CHUNK_SIZE = 1024 * 8  # Bytes per chunk
SLEEP_TIME = 0.005  # Seconds to wait between chunks

def create_test_file():
    # Ensure the directory exists first to avoid errors
    os.makedirs(os.path.dirname(FILE_NAME), exist_ok=True)
    
    if not os.path.exists(FILE_NAME):
        print(f"[*] Creating dummy file: {FILE_NAME}")
        with open(FILE_NAME, 'w') as f:
            f.write("This is a test of chunked transfer encoding.\n" * 50)
    else:
        print(f"[*] Using existing file: {FILE_NAME}")

def chunked_file_reader(file_path):
    with open(file_path, 'rb') as f:
        chunk_count = 0
        while True:
            chunk = f.read(CHUNK_SIZE)
            if not chunk:
                break
            
            chunk_count += 1
            print(f" -> Sending chunk {chunk_count}: {len(chunk)} bytes...")
            yield chunk
            
            # --- THE SLEEP IS ADDED HERE ---
            time.sleep(SLEEP_TIME) 

def run_test():
    create_test_file()
    
    print(f"[*] Sending {FILE_NAME} to {TARGET_URL} in chunks with {SLEEP_TIME}s delay...")
    
    try:
        headers = { 'Content-Type': 'binary' } 
        
        # 'data' accepts a generator for chunked transfer
        response = requests.post(
            TARGET_URL, 
            data=chunked_file_reader(FILE_NAME), 
            params={ 'path': TARGET_FILE },
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
