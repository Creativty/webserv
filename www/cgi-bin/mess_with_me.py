from os import close
from time import sleep

if __name__ == '__main__':
    close(0)
    close(1)
    close(2)
    sleep(4)
    exit(125)
