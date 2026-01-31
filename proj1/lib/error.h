// Copyright 2026 CSCE 311 Project 1
//
// Error helper API for Project 1.
// 
// Compile / link advice for this library:
// 
// C (student project written in C):
//   gcc -std=c11 --pedantic -Wall -Wextra -I.. -c main.c -o main.o
//   gcc -o proj1 main.o lib/proj1lib.a -pthread
// 
// C++ (student project written in C++):
//   g++ -std=c++20 --pedantic -Wall -Wextra -I.. -c main.cc -o main.o
//   g++ -o proj1 main.o lib/proj1lib.a -pthread
// 
// API guarantees:
//   - Both functions print a message to stderr, flush stderr, and then
//     terminate the process by calling exit(1).
//   - They do not return to the caller (noreturn).
//   - They are safe to call from any thread (the implementation uses stdio
//     which is thread-safe on POSIX); concurrent calls may interleave output.
//   - These helpers are not async-signal-safe (do not call from signal handlers
//     that require async-signal-safe functions).
//   - No global state is required or modified beyond writing to stderr.
//
// Usage:
//   Die("fatal error");
//   Dief("failed to open %s: %d", path, errno);
//
#ifndef PROJ1_LIB_ERROR_H_
#define PROJ1_LIB_ERROR_H_

#if defined(__cplusplus)
  #define NORETURN [[noreturn]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #define NORETURN _Noreturn
#else
  #define NORETURN
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Print the given null-terminated string to stderr (prefixed with "error: ")
// flush stderr, and terminate the process with exit status 1.
// *msg* may be NULL; in that case a generic message is printed.
// 
NORETURN void Die(const char *msg);

// Variadic version: format the message using printf-style semantics,
// print to stderr (prefixed with "error: "), flush, and exit(1).
// 
NORETURN void Dief(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif  // PROJ1_LIB_ERROR_H_
