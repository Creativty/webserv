from time import time
from random import randrange

import json

if __name__ == '__main__':
    data = {
        'name': 'John Doe',
        'age': int(randrange(start = 18, stop = 70)),
        'timestamp': float(time()),
        'interests': [ 'server', 'client', 'process', 'file' ],
        'counters': {
            'initial': randrange(start = 0, stop = 100),
            'current': randrange(start = 0, stop = 100),
        },
    }
    text = json.dumps(data)

    print(f"Content-Type: application/json", end = "\r\n")
    print(f"Content-Length: {len(text)}", end = "\r\n")
    print(end = "\r\n")
    print(text, end = "", flush = True)
