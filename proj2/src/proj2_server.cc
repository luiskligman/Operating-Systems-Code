// Copyright 2026 Luis Kligman

// Portable Operating System Interface (POSIX)
#include <unistd.h>

// C Headers
#include <pthread.h>
#include <csignal>
#include <cstring>

// C++ Headers
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <sys/socket.h>

#include "../lib/file_reader.h"
#include "../lib/domain_socket.h"
#include "../lib/sha_solver.h"
#include "../lib/thread_log.h"
#include "../lib/timings.h"  // Might not need

// Global stop flag
static volatile sig_atomic_t g_stop = 0;
// Upon signal being recieved, atomically change the stop variable to 1
void SigHandler(int) { g_stop = 1; }

struct WorkItem {
    std::string data;  // Raw Datagram bytes
};

struct ThreadPool {
    std::vector<pthread_t> threads;
    std::queue<WorkItem> queue;
    pthread_mutex_t mutex;  // Used to Protect the Queue
    pthread_cond_t cv; 
    bool stop;

    ThreadPool() : stop(false) {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cv, nullptr);
    }

    ~ThreadPool() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cv);
    }
};

void* Worker(void* arg);

// Spin up threads based on core count
void ThreadPool_Init(ThreadPool* pool) {
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    
    pool->threads.resize(num_threads);
    for (int i = 0; i < num_threads; ++i) 
        pthread_create(&pool->threads[i], nullptr, Worker, pool);
}

// Push a WorkItem onto the queue and wake one worker
void ThreadPool_Push(ThreadPool* pool, WorkItem item) {
    pthread_mutex_lock(&pool->mutex);
    pool->queue.push(std::move(item));
    pthread_cond_signal(&pool->cv);  // wake exactly one sleeping thread
    pthread_mutex_unlock(&pool->mutex);
}

// Tell all workers to stop and wait for them to finish
void ThreadPool_Shutdown(ThreadPool* pool) {
    pthread_mutex_lock(&pool->mutex);
    pool->stop = true;
    pthread_cond_broadcast(&pool->cv);  // Wake all sleeping workers
    pthread_mutex_unlock(&pool->mutex);

    for (pthread_t& t : pool->threads)
        pthread_join(t, nullptr);

}

void HandleRequest(const WorkItem& item) {
    const char* ptr = item.data.data();

    // Parse reply path
    uint32_t reply_path_len;
    std::memcpy(&reply_path_len, ptr, 4);
    ptr += 4;

    std::string reply_path(ptr, reply_path_len);
    ptr += reply_path_len;

    // Parse file count
    uint32_t file_count;
    std::memcpy(&file_count, ptr, 4);
    ptr += 4;

    // Parse each file's path and row count
    std::vector<std::string> file_paths;
    std::vector<uint32_t> row_counts;
    uint32_t max_rows = 0;

    for (uint32_t i = 0; i < file_count; ++i) {
        uint32_t path_len;
        std::memcpy(&path_len, ptr, 4);
        ptr += 4;

        std::string path(ptr, path_len);
        ptr += path_len;

        uint32_t row_count;
        std::memcpy(&row_count, ptr, 4);
        ptr += 4;

        file_paths.push_back(std::move(path));
        row_counts.push_back(row_count);

        if (row_count > max_rows) { max_rows = row_count; }
    }
        
    // Checkout solvers first, then readers
    //proj2::SolverHandle solvers = proj2::ShaSolvers::Checkout(max_rows);
    proj2::ReaderHandle readers = proj2::FileReaders::Checkout(file_count, nullptr); 

    // Process files
    std::vector<std::vector<proj2::ReaderHandle::HashType>> file_hashes(file_count);  // Pre-size via file_count
    readers.Process(file_paths, row_counts, &file_hashes);

    // Checkin readers first, then solvers
    proj2::FileReaders::Checkin(std::move(readers));
    //proj2::ShaSolvers::Checkin(std::move(solvers));

    // Flatten hashes into one contigious string, ordered by file
    std::string response;
    for (const auto& file : file_hashes)
        for (const auto& hash : file)
            response.append(hash.data(), 64);
    
    // Stream response back to the client
    proj2::UnixDomainStreamClient client(reply_path);
    client.Init();
    // .data() returns a const char* which implicitly converts to const void*
    client.Write(response.data(), response.size());
    

}

// Each thread runs this forever until stop is set
void* Worker(void* arg) {
    ThreadPool* pool = static_cast<ThreadPool*>(arg);

    while (true) {
        pthread_mutex_lock(&pool->mutex);

        // Sleep until there is work or we are shutting down
        while (pool->queue.empty() && !pool->stop)
            pthread_cond_wait(&pool->cv, &pool->mutex);  // Release mutex while waiting for cv 

        // Shutdown path, no more work
        if (pool->stop && pool->queue.empty()) {
            pthread_mutex_unlock(&pool->mutex);
            return nullptr;
        }

        WorkItem item = std::move(pool->queue.front());
        pool->queue.pop();
        pthread_mutex_unlock(&pool->mutex);
        
        HandleRequest(item);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <socket_path> <total_readers> <total_solvers>" << std::endl;
        return 1;
    }

    // Create signal handlers
    signal(SIGINT, SigHandler);
    signal(SIGTERM, SigHandler);

    // Init the Resource Pools (setting internal values)
    proj2::FileReaders::Init(atoi(argv[2]));  // Set total_available
    proj2::ShaSolvers::Init(atoi(argv[3]));  // Set total_available

    // Init thread pool
    ThreadPool pool;
    ThreadPool_Init(&pool);

    // Bind Unix Domain Datagram Socket Using <socket_path>
    proj2::UnixDomainDatagramEndpoint socket(argv[1]);  
    socket.Init();  // Bind

    // RecvFrom is a blocking call, tell the OS to automatically unblock after 1 second even if no datagram arrived
    struct timeval tv {1, 0};
    setsockopt(socket.socket_fd(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Blocking loop, wait for datagrams and hand off to workers
    while (!g_stop) {
        std::string peer_path;
        std::string data = socket.RecvFrom(&peer_path, 4096);  // 4 KB, default memory page size
        if (data.empty()) 
            continue;
        ThreadPool_Push(&pool, {std::move(data)});
    }

    ThreadPool_Shutdown(&pool);

    return 0;
}

