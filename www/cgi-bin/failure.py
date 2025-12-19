from os import environ as envp
from sys import argv
from time import sleep

if __name__ == '__main__':
    print("<!DOCTYPE html>")
    print("<html>")
    print("<body>")
    print("  <h1>Failure</h1>")
    print("  <h2>argv</h2>")
    print("  <ul>")
    for arg in argv:
        print(f"    <li>\"{arg}\"</li>")
    print("  </ul>")
    print("  <hr>");
    print("  <h2>envp</h2>")
    exit(1)
    print("  <ul>")
    for name in envp:
        value = envp[name]
        print(f"    <li>{name} = \"{value}\"</li>")
    print("  </ul>")
    print("</body>")
    print("</html>")
