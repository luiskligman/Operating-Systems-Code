// Copyright 2026 CSCE 311 Project 1
//
// Thread-safe logging to stdout for Project 1.
//
// I am so tired of comments at this point. If you do not know how to use this,
// checkout printf from cstdio. It works just like that, but is thread-safe.
#ifndef PROJ1_THREAD_LOG_H_
#define PROJ1_THREAD_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

// ThreadLog(fmt, ...)
//   Prints a single formatted line to stdout. This function is thread-safe:
//   multiple threads may call ThreadLog concurrently and their entire
//   formatted messages will not interleave.
//
// Notes:
//  - This is NOT async-signal-safe. Do not call from signal handlers.
//  - Messages should be reasonably short (a few KB max); vfprintf does the
//    formatting into libc buffers.
//
void ThreadLog(const char *fmt, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PROJ1_THREAD_LOG_H_ */
//
