// Copyright 2026 Luis Kligman

// Portable Operating System Interface (POSIX)
#include <unistd.h>

// C Headers
#include <pthread.h>
#include <stdint.h>

// C++ Headers
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "../lib/cli_parser.h"
#include "../lib/error.h"
#include "../lib/sha256.h"
#include "../lib/thread_log.h"
#include "../lib/timings.h"

/*  Example Interaction
 * ./proj1 --all --timeout 3000 < data/small.txt
 * Enter max threads (1-8): 4
 * THREAD OUTPUT :: 'started', 'returned', 'completed', etc.
 */

// static int that corresponds to the max k threads the user selected
static int max_threads;

// struct to store the input from the file
struct Task {
    // this id is given by input file
    // (different from id in thread_info struct ** but should be equal)
    std::string id;
    std::string name;
    int amount;
};

// struct to properly store info relating to each thread
// struct allows user to quickly add fields / remove fields
struct Thread_info {
    int id;
    volatile int exec_mode;
    /*
     * negative values :: exit
     * zero :: sleep (wait)
     * positive values :: execute
     */

    int max_threads;  // user selected number of threads
    int nrows;  // number of tasks
    Task* tasks;  // pointer to the vector of tasks
    std::string* results;  // pointer to results array

    uint32_t timeout_ms;
    uint64_t start_time_ms;

    Thread_info* next_thread;
};

// the work that each thread will perform
void *worker(void *arg) {
    Thread_info* info = static_cast<Thread_info*>(arg);

    // threads in thread pool must sleep until exec_mode != 0
    while (info->exec_mode == 0) {
        Timings_SleepMs(1);
    }

    // threads with negative exec_mode must exit immediately
    if (info->exec_mode < 0) {
        ThreadLog("[thread %d] returned\n", info->id);
        return NULL;
    }

    ThreadLog("[thread %d] started\n", info->id);

    // record when thread starts work
    uint64_t start_time = Timings_NowMs();

    int thread_index = info->id - 1;  // convert to 0 based for array access

    // thread i (1-indexed) processes rows: i-1, (i-1)+k, (i-1)+2k
    // treat the task array as 0 indexed
    for (int r = 0; r * info->max_threads + thread_index < info->nrows; r++) {
        const int row_index = thread_index + r * info->max_threads;

        // Check timeout before processing each row
        if (Timings_NowMs() - start_time > info->timeout_ms) {
            ThreadLog("[thread %d] timeout\n", info->id);
            ThreadLog("[thread %d] returned\n", info->id);

            // release next if exec_mode == thread
            if (info->exec_mode == 3 && info->next_thread != NULL)
                info->next_thread->exec_mode = 3;

            return NULL;
        }

        // Get the task at this row
        Task* task = &info->tasks[row_index];

        // Compute the sha256 hash
        char hex_output[65] = {0};
        ComputeIterativeSha256Hex(
            reinterpret_cast<const uint8_t*>(task->name.data()),
            task->name.size(),
            task->amount,
            hex_output);


        info->results[row_index] = hex_output;

        ThreadLog("[thread %d] completed row %d\n", info->id, row_index);
    }

    // release next if exec_mode == 3
    if (info->exec_mode == 3 && info->next_thread != NULL)
        info->next_thread->exec_mode = 3;

    ThreadLog("[thread %d] returned\n", info->id);
    return NULL;
}

int main(int argc, char *argv[]) {
    // figure out how many online threads the host computer currently has
    int16_t online_threads = sysconf(_SC_NPROCESSORS_ONLN);
    ThreadLog("online threads: %ld\n", online_threads);

    // create two separate vectors the size of the number of online_threads
    // keep track of threads and accompanying info
    std::vector<pthread_t> threads(online_threads);
    std::vector<Thread_info> info(online_threads);

    // parse argv to separate potential flags
    CliMode mode;
    uint32_t timeout_ms;  // default timeout is 5,000 (5s)
    CliParse(argc, argv, &mode, &timeout_ms);
    ThreadLog("mode: %u, timeout: %d\n", mode, timeout_ms);


    // capture the number of tasks which is the first
    // row in the piped input file
    int n;
    std::cin >> n;

    // create a vector of results
    std::vector<std::string> results(n);

    // create a vector of tasks
    std::vector<Task> tasks(n);

    // record global start time
    uint64_t global_start = Timings_NowMs();

    // take row input from the piped input file
    for (int i=0; i < n; ++i) {
        std::cin >> tasks[i].id >> tasks[i].name >> tasks[i].amount;
    }

    // after reading all the input from STDIN
    // (via I/O redirect from a file), prompts the user
    // (via dev/tty) for a number, k, of threads to use for this execution
    ThreadLog("Enter max threads (1 - %ld): \n", online_threads);
    std::ifstream tty_in("/dev/tty");
    if (tty_in) {
        tty_in >> max_threads;
    }

    // initialize all threads with shared data
    for (int i = 0; i < online_threads; i++) {
        info[i].id = i + 1;
        info[i].exec_mode = 0;
        info[i].max_threads = max_threads;
        info[i].nrows = n;
        info[i].tasks = tasks.data();  // pointer to task 0 in the array
        info[i].results = results.data();  // pointer to result 0 in the array
        info[i].timeout_ms = timeout_ms;
        info[i].start_time_ms = global_start;
        info[i].next_thread = (i + 1 < online_threads ? &info[i + 1] : NULL);

        pthread_create(&threads[i], NULL, worker, &info[i]);
    }

    // release the threads depending on the chosen mode
    if (mode == 0) {  // --all: release all max_threads at once
        for (int i = 0; i < online_threads; i++) {
            if (i < max_threads && i < n)
                info[i].exec_mode = 1;
            else
                info[i].exec_mode = -1;
        }
    } else if (mode == 1) {  // --rate: release on thread per millisecond
        for (int i = 0; i < online_threads; i++) {
            if (i < max_threads && i < n) {
                info[i].exec_mode = 2;
                Timings_SleepMs(1);
            } else {
                info[i].exec_mode = -1;
            }
        }
    } else if (mode == 2) {  // --thread: each thread releases the next
        // Release only the first thread; it will release the next
        for (int i = 0; i < online_threads; i++) {
            if (i == 0 && i < max_threads && i < n)
                info[i].exec_mode = 3;
            else if (i >= max_threads || i >= n)
                info[i].exec_mode = -1;
            // other threads will stay waiting at exec_mode == 0
            // waiting to be release by prev thread
        }
    }

    // wait for all threads to complete
    for (int i = 0; i < online_threads; i++)
        pthread_join(threads[i], NULL);


    ThreadLog("Thread       Start       Encryption\n");
    for (int i = 0; i < n; ++i) {
        int thread_number = (i % max_threads) + 1;
        ThreadLog("%d           %s          %s\n",
            thread_number, tasks[i].name.c_str(), results[i].c_str());
    }

    return 0;
}
