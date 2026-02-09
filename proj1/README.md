# Project 1: Scheduling Threads

## File Descriptions

### main.cc

The primary source file containg the complete implementation of the multi-threaded scheduling program. The file includes:

- Global variable `max_threads` to store the user selected thread count
- `Task` struct to represent input data (id, name, amount)
- `Thread_info` struct to manage thread state and shared data between threads
- `worker()` function that defines thread behavior and work distribution
- `main()` function that orchestrates thread creation, scheduling, and result reporting

### main.h

Optional header file for function declarations and struct defintions. Currently I chose not to use it in this implementation as all code is self contained in `main.cc`

## Design Choices

### Thread Pool Architecture

The program creates a thread pool equal to the number of CPU cores (`_SC_NPROCESSORS_ONLN`). All threads are created at startup in a "paused/sleep" statem waiting for activation.

### Execution Control via exec_mode

Each thread's behavior is controled by a volatile int `exec_mode` field in the `Thread_info` struct:

- **Negative values**: Thread exits immediately
- **Zero**: Thread sleeps/waits in a while loop (initial paused state)
- **Positive values**: Thread executes its assigned work (1 = --all, 2 = --rate, 3 = --thread)

This design choice allows for simple and flexible thread scheduling.

### Work Distribution Strategy

Threads process input rows using a round robin distribution pattern:

- Thread `i` (1 indexed) processes rows: `(i-1)`, `(i-1) + k`, `(i-1) + 2k`, `...`
- Where `k` is the user selected number of threads
- This ensures even distribution of work across active threads

### Three Release Modes

**--all Mode**: All `k` threads have their `exec_mode` set to positive simultaneously, allowing concurrent startup, it is intentional that is may allow for race conditions.

**--rate Mode**: The main thread sets `exec_mode` for one thread at a time with 1ms delays between releases, creating a staggered startup pattern.

**--thread Mode**: Only the first thread is initially released. Each thread releases the next thread upon completion or by timeout, by setting its `next_thread->exec_mode`, creating a cascading release pattern.

### Timeout Handling

Each thread check elapsed time before processing each row. If the timeout is exceeded, the tread logs a timeout message and exits. This ensures the program terminates within the specified time bound.

### Data Sharing Without Synchronization

Shared Data (tasks array, results array, task count) is stored in the `Thread_info` struct and passed to each thread via pointers. Race conditions are intentionally left unresolved as per the project requirements, demonstrating the need for proper synchronization in real world applications.

### Logging and Output

The program uses the provided `ThreadLog()` function for thread event logging (started, completed row, returned) and produces a final report showing which thread processed each input row and the computed SHA-256 hash results. Through running the `ThreadLog()` on my personal linux computer, I found that it would include extra new line characters, but instead of using `printf()`, I stuck with `ThreadLog()` due to its thread safety.

## Building and Running

```bash
make
./bin/proj1 --all --timeout 3000 < ./dat/small.txt>
```

When prompted, enter the desired number of threads (1 to number of CPU cores).
