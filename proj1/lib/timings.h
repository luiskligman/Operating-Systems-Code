// Copyright SP26 CSCE311 Project 1
// timings.h -- small timing helpers for Project 1
// 
// Guarantees / design:
//  - Uses a monotonic clock (CLOCK_MONOTONIC) to avoid jumps from system time changes.
//  - Time values are expressed in milliseconds (uint64_t).
//  - Sleep uses nanosleep and handles EINTR by retrying; it is not busy-waiting.
//  - API is C and C++ compatible and does not allocate memory.
// 
// Typical usage:
//   Timings_t start = Timings_NowMs();
//   Timings_SleepMs(TIMINGS_MS_1);   // sleep 1 ms
//   if (Timings_TimeoutExpired(start, 5000)) { ... }
// 

#ifndef PROJ1_TIMINGS_H_
#define PROJ1_TIMINGS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t Timings_t;

// Return current time from a monotonic clock in milliseconds.
Timings_t Timings_NowMs(void);


// Sleep approximately 'ms' milliseconds. This will call nanosleep and retry on
// EINTR until the requested time has elapsed (or until interrupted by a fatal
// signal). If you do not know about EINT, you should ask me or look it up.
// Passing 0 returns immediately.
// 
void Timings_SleepMs(uint32_t ms);

// Return nonzero (true) if (NowMs() - start_ms) >= timeout_ms, zero (false)
// otherwise
// Semantics: treat timeout_ms==0 as "immediately expired" (i.e., returns true).
// 
int Timings_TimeoutExpired(Timings_t start_ms, uint32_t timeout_ms);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // PROJ1_LIB_TIMINGS_H_
