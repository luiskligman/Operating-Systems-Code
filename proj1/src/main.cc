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
struct thread_info {
    int id;
    volatile int exec_mode;
    /*
     * -2 == exit
     * -1 == sleep
     * 0 == all
     * 1 == rate
     * 2 == thread
     */
    Task task;
};

// the work that each thread will perform
void *worker(void *arg) {
    thread_info* info = static_cast<thread_info*>(arg);

    // threads in thread pool must sleep until specific
    while (info->exec_mode == -1) {
        //printf("slept\n");
        Timings_SleepMs(3000);
    }

    // threads with -2 exec_mode must exit immediately
    if (info->exec_mode == -2) {
        return NULL;
    }

    // too many threads spun compared to users max threads value
    if (max_threads < info->id) {
        printf("id greater than max_threads: %d\n", info->id);
        return NULL;
    }

    // initialize output char vector
    char out_hex[65] = {0};

    // if exec_mode == 0 (--all) fire the thread as soon as this is reached
    // no hold necessary
    if (info->exec_mode == 0) {
        // .data() will point to the seed bytes as requested by function definition
        ComputeIterativeSha256Hex(
            reinterpret_cast<const uint8_t*>(info->task.name.data()),
            info->task.name.size(),
            info->task.amount,
            out_hex);
    }


    printf("id: %d, sha256_output: %s\n", info->id, out_hex);

    return NULL;
}

int main(int argc, char *argv[]) {
    // figure out how many online threads the host computer currently has
    long online_threads = sysconf(_SC_NPROCESSORS_ONLN);
    printf("online threads: %ld\n", online_threads);

    // create two separate vectors the size of the number of online_threads
    // keep track of threads and accompanying info
    std::vector<pthread_t> threads(online_threads);
    std::vector<thread_info> info(online_threads);

    // spin up threads
    for (int i = 0; i < online_threads; i++) {
        info[i].id = i + 1;  // threads are indexed starting at 1
        info[i].exec_mode = -1;  // make sure all threads know how they should execute
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

    // take row input from the piped input file
    for (int i=0; i < n; ++i) {
        std::cin >> info[i].task.id >> info[i].task.name >> info[i].task.amount;
    }

    // after reading all the input from STDIN (via I/O redirect from a file), prompts the user
    // (via dev/tty) for a number, k, of threads to use for this execution
    std::cerr << "enter how many k threads you wish to use for this execution\n";
    std::ifstream tty_in("/dev/tty");
    // if (tty_in) {
    //     int var;
    //     tty_in >> var;
    // }
    if (tty_in) {
        tty_in >> max_threads;
    }
    // max_threads = 5;

    // update the exec_mode
    for (int i = 0; i < online_threads; ++i) {
        if (i < max_threads) {
            info[i].exec_mode = mode;
        } else {
            info[i].exec_mode = -2;  // tell the thread to exit immediately
        }
    }






    //
    // pthread_t p1;
    // thread_info info;
    // info.id = 1;
    // pthread_create(&p1, NULL, worker, &info);


    // make sure all threads are complete (using join)
    for (int i = 0; i < online_threads; i++) {
        pthread_join(threads[i], NULL);  // waits for the thread to complete
    }

    return 0;
}
