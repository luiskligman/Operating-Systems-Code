// Copyright SP26 CSCE311 Project 2
//
// Thread-safe logging to stdout and stderr for Project 2.
//
// To use this, read up on printf from cstdio. It works just like that, but is
// thread-safe.
#ifndef PROJ2_THREAD_LOG_H_
#define PROJ2_THREAD_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

// ThreadLog(fmt, ...)
// ThreadErr(fmt, ...)
//   Each function uses a producer and consumer to avoid blocking calling
//   threads for prints, i.e., a thread calling either function will only block
//   long enough to add the char string to a queue for printing.
//
// Notes:
//  - This is NOT async-signal-safe. Do not call from signal handlers.
//  - Messages should be reasonably short (a few KB max); vfprintf does the
//    formatting into libc buffers.
//
void ThreadLog(const char *fmt, ...);
void ThreadErr(const char *fmt, ...);

void StopLog();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif   // PROJ2_LIB_THREAD_LOG_H_
