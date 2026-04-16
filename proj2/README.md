# Project 2 - CSCE 311 - Luis Kligman

## Overview
A concurrent server that receives file-hashing requests from clients over Unix domain datagram sockets, processes them using a managed pool of file readers and SHA solvers, and streams results back over Unix doamin stream sockets.

## Project Design Choices
### Thread Pool
A fuxed-size thread pool is initialized at startup with one thread per online CPU core `sysconf(_SC_NPROCESSORS_ONLN)`. A single mutex-protected `std::queue` holds incoming `WorkItems`. The main thread receives datagrams and pushes them onto the queue. Worker threads pull and process them. A condition variable is used so workers sleep when the queue is empty rather than busy-waiting. On shutdown, `pthread_cond_broadcast` wakes all sleeping workers so they can exit cleanly, and `pthread_join` waits for each to finish.

### Main Receive Loop
The main thread runs a blockingg loop calling `RecvFrom` on the datagram socket. The socket unblocks periodically, allowing the loop to check the `g_stop` flag and exit promptly on SIGINT/SIGTERM rather than hanging until the next datagram arrives. 

### Signal Handling
SIGINT and SIGTERM are handled by a signal handler that sets a `volatile sig_atomic_t` flag `g_stop`. The main loop checks this flag each iteration and begins shutdown when set.

### Binary Parsing
Datagrams are parsed using `std::memcpy` into `uint32_t` fields follwing the length-prefixed framing defined by the protocol.

### Resouce Management and Deadlock Prevention
`FileReaders::CEhckout(file_count, nullptr)` is called to acquire the necessary reader slots. The library manages SHA solver acquisition internally during `Process()`, it checks out the appropriate number of solver slots per file and releases them before moving to then next. By allowing the library to manage solvers, resource acquisition always follows a consistent ordering.

### Response
After `Process()` fills the per-file hash vectors, the hashes are flattened into a single contiguous byte string ordered by file. A `UnixDomainStreamClient` connects to the client's reply endpoint and streams teh full response in one write.

