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

/*
 * static int to hold the different mode the threads should be released in
 * 0 == hold
 * 1 == all
 * 2 == rate
 * 3 == thread
 */
static volatile int release_threads;

// struct to store the input from the file
struct Task {
    std::string id;
    std::string name;
    int amount;
};

// struct to properly store info relating to each thread
// struct allows user to quickly add fields / remove fields
struct thread_info {
    int id;
};

// the work that each thread will perform
void *worker(void *arg) {
    thread_info* info = static_cast<thread_info*>(arg);

    // too many threads spun compared to users max threads value
    if (max_threads < info->id) {
        printf("id greater than max_howthreads: %d\n", info->id);
        return NULL;
    }

    // threads in thread pool must sleep until specific
    // while (release_threads == 0) {
    //     printf("slept\n");
    //     // Timings_SleepMs(3000);
    // }

    printf("id: %d\n", info->id);

    return NULL;
}

int main(int argc, char *argv[]) {
    // parse argv to separate potential flags
    CliMode mode;
    uint32_t timeout_ms;  // default timeout is 10,000 (10s)
    CliParse(argc, argv, &mode, &timeout_ms);
    printf("mode: %u, timeout: %d\n", mode, timeout_ms);

    // capture the number of tasks which is the first row in the piped input file
    int n;
    std::cin >> n;

    // init release_threads to 0 so that the threads will sit idle in the thread pool
    release_threads = 0;

    // create a vector of size n (n is known)
    std::vector<Task> tasks(n);
    for (int i=0; i < n; ++i) {
        std::cin >> tasks[i].id >> tasks[i].name >> tasks[i].amount;
    }

    // after reading all the input from STDIN (via I/O redirect from a file), prompts the user
    // (via dev/tty) for a number, k, of threads to use for this execution
    std::cerr << "enter how many k threads you wish to use for this execution\n";
    std::ifstream tty_in("/dev/tty");
    // if (tty_in) {
    //     int var;
    //     tty_in >> var;
    // }
    // if (tty_in) {
    //     tty_in >> max_threads;
    // }
    max_threads = 5;

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
        pthread_create(&threads[i], NULL, worker, &info[i]);
    }

    release_threads = 1;


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
