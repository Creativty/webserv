from os import environ as envp
from sys import argv
from time import sleep

if __name__ == '__main__':
    sleep(2)
    print("<!DOCTYPE html>")
    print("<html>")
    print("<body>")
    sleep(2)
    print("  <h1>Slow</h1>", flush = True)
    print("  <h2>argv</h2>")
    print("  <ul>")
    for arg in argv:
        print(f"    <li>\"{arg}\"</li>")
    print("  </ul>", flush = True)
    sleep(2)
    print("  <hr>");
    print("  <h2>envp</h2>")
    print("  <ul>")
    for name in envp:
        value = envp[name]
        print(f"    <li>{name} = \"{value}\"</li>")
    print("  </ul>", flush = True)
    sleep(2)
    print("</body>")
    print("</html>")
