#include "../lib/cli_parser.h"
#include "../lib/error.h"
#include "../lib/sha256.h"
#include "../lib/thread_log.h"
#include "../lib/timings.h"

#include <string>
#include <pthread.h>
#include <fstream>
#include <iostream>

// Portable Operating System Interface (POSIX)
#include <unistd.h>

struct Task {
    std::string id;
    std::string name;
    int amount;
};

// the work that each thread will perform
void *worker(void *arg) {
    return NULL;
}

int main(int argc, char *argv[]) {
    // capture the number of tasks which is the first row in the piped input file
    int n;
    std::cin >> n;

    // create a vector of size n (n is known)
    std::vector<Task> tasks(n);
    for (int i=0; i < n; ++i) {
        std::cin >> tasks[i].id >> tasks[i].name >> tasks[i].amount;
    }

    // after reading all the input from STDIN (via I/O redirect from a file), prompts the user
    // (via dev/tty) for a number, k, of threads to use for this execution
    std::cerr << "enter how many k threads you wish to use for this execution\n";
    std::ifstream tty_in("/dev/tty");
    if (tty_in) {
        int var;
        tty_in >> var;
    }

    // figure out how many online threads the host computer currently has
    long online_threads = sysconf(_SC_NPROCESSORS_ONLN);

    printf("online threads: %ld\n", online_threads);




    return 0;
}
