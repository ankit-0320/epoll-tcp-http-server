import socket
import threading

def client():
    s = socket.socket()
    s.connect(("127.0.0.1", 8080))
    s.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
    s.recv(1024)
    s.close()

threads = []
for _ in range(200):
    t = threading.Thread(target=client)
    t.start()
    threads.append(t)

for t in threads:
    t.join()

print("HTTP load test completed")

