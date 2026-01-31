// Copyright 2026 CSCE 311 Project 1
//
// Minimal command-line parser for Project 1.
//
// Purpose:
//   - Parse two runtime values from argc/argv:
//       1) release mode: --all | --rate | --thread
//       2) per-thread timeout (optional): --timeout <integer>
//   - Supply results via output parameters (no heap, no struct).
//   - On malformed input the parser will terminate the program (calls Die/Dief).
//
// Notes:
//   - Default timeout_ms = 3000 (3 seconds).
//   - This header is C and C++ compatible.
//
// Usage example:
//   CliMode mode;
//   uint32_t timeout_ms;
//   CliParse(argc, argv, &mode, &timeout_ms);
//

#ifndef PROJ1_CLI_PARSER_H_
#define PROJ1_CLI_PARSER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Thread release modes */
typedef enum {
  CLI_MODE_ALL = 0,   /* release all threads immediately */
  CLI_MODE_RATE,      /* main thread releases threads at 1 ms intervals */
  CLI_MODE_THREAD     /* threads release other threads (racy) */
} CliMode;

typedef uint32_t Time_t;

/* Parse command-line arguments into output parameters.
 *
 * Accepted invocation forms (examples):
 *   ./proj1 --all
 *   ./proj1 --rate
 *   ./proj1 --thread
 *   ./proj1 --all --timeout 5000
 *
 * Output:
 *   *out_mode       receives the parsed mode
 *   *out_timeout_ms receives the parsed timeout in milliseconds (or default)
 *
 * Behavior:
 *   - Default timeout if not provided: 10000 (10s).
 *   - If arguments are malformed or an unknown flag is given, this function
 *     will print an error and terminate the program (via Die/Dief).
 *   - The function does not return an error code by design; callers may assume
 *     that on return both out parameters are valid.
 *
 * Implementation notes:
 *   - Accepts flags in any order.
 *   - If more than one mode flag is provided, this is an error.
 *   - Timeout must be a non-negative integer representable in uint32_t.
 */
void CliParse(int argc, char *argv[], CliMode *out_mode, uint32_t *out_timeout_ms);

/* Constant: default per-thread timeout (ms) */
enum { CLI_DEFAULT_TIMEOUT_MS = 5000 };

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PROJ1_CLI_PARSER_H_ */

