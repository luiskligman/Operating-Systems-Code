#include "../lib/cli_parser.h"
#include "../lib/error.h"
#include "../lib/sha256.h"
#include "../lib/thread_log.h"
#include "../lib/timings.h"

#include <string>
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdint.h>

// Portable Operating System Interface (POSIX)
#include <unistd.h>


/*  Example Interaction
 * ./proj1 --all --timeout 3000 < data/small.txt
 * Enter max threads (1 - 8) : 4
 * THREAD OUTPUT :: 'started' , 'returned' , 'completed' , etc
 */

// static int that corresponds to the max k threads the user selected
static int max_threads;


// struct to store the input from the file
struct Task {
    // this id is given by input file (different from id in thread_info struct ** but should be equal)
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

    Task task;

    // initialize output char vector
    char out_hex[65] = {0};
};

// the work that each thread will perform
void *worker(void *arg) {
    Thread_info* info = static_cast<Thread_info*>(arg);

    // threads in thread pool must sleep until exec_mode != 0
    while (info->exec_mode == 0) {
        //printf("slept\n");
        Timings_SleepMs(5);
    }

    // threads with negative exec_mode must exit immediately
    if (info->exec_mode < 0) {
        printf("[thread %d] returned\n", info->id);
        return NULL;
    }

    printf("[thread %d] started\n", info->id);


    // execute tasks if exec_mode is positive
    if (info->exec_mode > 0) {
        // .data() will point to the seed bytes as requested by function definition
        ComputeIterativeSha256Hex(
            reinterpret_cast<const uint8_t*>(info->task.name.data()),
            info->task.name.size(),
            info->task.amount,
            info->out_hex);
    }

    printf("[thread %d] completed row %s\n", info->id, info->task.id.c_str());

    return NULL;
}

int main(int argc, char *argv[]) {
    // figure out how many online threads the host computer currently has
    long online_threads = sysconf(_SC_NPROCESSORS_ONLN);
    printf("online threads: %ld\n", online_threads);

    // create two separate vectors the size of the number of online_threads
    // keep track of threads and accompanying info
    std::vector<pthread_t> threads(online_threads);
    std::vector<Thread_info> info(online_threads);

    // spin up threads
    for (int i = 0; i < online_threads; i++) {
        info[i].id = i + 1;  // threads are indexed starting at 1
        info[i].exec_mode = 0;  // make sure all threads know how they should execute
        pthread_create(&threads[i], NULL, worker, &info[i]);
    }

    // parse argv to separate potential flags
    CliMode mode;
    uint32_t timeout_ms;  // default timeout is 5,000 (5s)
    CliParse(argc, argv, &mode, &timeout_ms);
    printf("mode: %u, timeout: %d\n", mode, timeout_ms);


    // capture the number of tasks which is the first row in the piped input file
    int n;
    std::cin >> n;

    // keep track of the tasks that we process from the input file
    int tot_tasks = 0;

    // create a vector of tasks
    std::vector<Task> tasks(n);

    // take row input from the piped input file
    for (int i=0; i < n; ++i) {
        std::cin >> tasks[i].id >> tasks[i].name >> tasks[i].amount;
    }

    // after reading all the input from STDIN (via I/O redirect from a file), prompts the user
    // (via dev/tty) for a number, k, of threads to use for this execution
    // std::cerr << "Enter max threads (1 - %d): \n", online_threads;
    printf("Enter max threads (1 - %ld): \n", online_threads);
    std::ifstream tty_in("/dev/tty");
    if (tty_in) {
        tty_in >> max_threads;
    }

    // release the threads depending on the mode selected
    for (int i = 0; i < online_threads; ++i) {
        if (max_threads <= i || n <= i) {
            info[i].exec_mode = -1;
        } else if (mode == 0) {  // flag: --all
            info[i].task = tasks[i];  // assign the task to thread
            info[i].exec_mode = 1;
        } else if (mode == 1) {  // flag: --rate
            Timings_SleepMs(1);  // stops threads from releasing by 1 Ms
            info[i].task = tasks[i];  // assign the task to thread
            info[i].exec_mode = 1;
        } else if (mode == 2) {  // flag: --thread
            if (i > 0)   // immediately release the first thread, but wait for it to return before firing next
                pthread_join(threads[i-1], NULL);  // wait for prev thread to complete
            info[i].task = tasks[i];  // assign the task to thread
            info[i].exec_mode = 1;

        }
    }

    // make sure all threads are complete (using join)
    for (int i = 0; i < online_threads; i++) {
        pthread_join(threads[i], NULL);  // waits for the thread to complete
    }

    printf("Thread       Start       Encryption\n");
    for (int i = 0; i < max_threads; ++i) {
        printf("%d           %s          %s\n",
            info[i].id, info[i].task.name.c_str(), info[i].out_hex);
    }

    return 0;
}
