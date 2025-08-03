import requests
import time
from concurrent.futures import ThreadPoolExecutor

URL = "http://localhost:8080/"
NUM_REQUESTS = 20

def make_request(i):
    try:
        response = requests.get(f"{URL}{i}", timeout=5)
        return f"[{i}] Status: {response.status_code} | Length: {len(response.content)}"
    except Exception as e:
        return f"[{i}] Error: {e}"

def main():
    start = time.time()
    with ThreadPoolExecutor(max_workers=NUM_REQUESTS) as executor:
        futures = [executor.submit(make_request, i) for i in range(1, NUM_REQUESTS + 1)]
        for future in futures:
            print(future.result())
    print(f"\nTotal time: {time.time() - start:.2f} seconds")

if __name__ == "__main__":
    main()

