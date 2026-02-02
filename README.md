# epoll-tcp-http-server

A high-performance TCP/HTTP server written in C++ using Linux `epoll`,
non-blocking sockets, and a fixed-size thread pool.  
Designed to demonstrate scalable I/O multiplexing and systems-level networking.

---

## Features

- Non-blocking TCP server using `epoll`
- Thread pool for concurrent request handling
- Basic HTTP request parsing (GET support)
- Atomic active-client metrics
- Graceful shutdown using SIGINT (Ctrl+C)
- Tested with `curl` and `netcat`

---

## Architecture

                        ┌───────────────┐
                        │   Client(s)   │
                        │ (curl / nc)   │
                        └───────┬───────┘
                                │
                                ▼
                      ┌───────────────────┐
                      │   TCP Socket      │
                      │  (Non-Blocking)   │
                      └───────┬───────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │        epoll           │
                    │  (Event Multiplexer)   │
                    └───────┬───────────────┘
                            │
           ┌────────────────┴────────────────┐
           │                                 │
           ▼                                 ▼
 ┌───────────────────┐           ┌───────────────────┐
 │  Accept New Conn  │           │  Client Read Event │
 │                   │           │                   │
 └─────────┬─────────┘           └─────────┬─────────┘
           │                                 │
           ▼                                 ▼
┌────────────────────┐          ┌────────────────────┐
│ Active Client +1   │          │   Thread Pool       │
│ (Atomic Counter)   │          │  (Worker Threads)   │
└────────────────────┘          └─────────┬──────────┘
                                           │
                                           ▼
                                ┌────────────────────┐
                                │  HTTP Parsing       │
                                │  & Response Write  │
                                └─────────┬──────────┘
                                           │
                                           ▼
                                ┌────────────────────┐
                                │ Active Client -1   │
                                │  + Socket Close    │
                                └────────────────────┘



---

## Build & Run

### Requirements
- Linux (tested on Ubuntu via WSL)
- g++ (C++17)
- make

### Build
bash h
make

### Run
./tcp_server

Usage Examples
## Using curl
curl http://localhost:8080

Expected response:
Hello from TCP Server

## Using netcat
echo "hello" | nc 127.0.0.1 8080

### Testing
python3 tests/load_test.py

