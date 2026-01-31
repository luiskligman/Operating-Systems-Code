// Copyright 2026 CSCE 311-001,002
//
// SHA-256 helper API for Project 1.
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
// SHA-256 specification (reference):
//   FIPS PUB 180-4: Secure Hash Standard (SHS)
//   https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf
//   Note:
//     The SHA-256 algorithm produces a 256-bit (32-byte) digest.
//     When encoded as hexadecimal, this corresponds to 64 ASCII characters.
// 
// API guarantees:
//   - No global state is used by these functions.
//   - All output buffers must be provided by the caller as output parameters.
//
#ifndef PROJ1_INCLUDE_SHA256_H_
#define PROJ1_INCLUDE_SHA256_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Sha256HexEncode:
//   Hex-encode `in` (length `inlen` bytes) into `out`.
//   `out` must have room for (inlen*2 + 1) bytes. Output is null-terminated.
// 
void Sha256HexEncode(const uint8_t *in, size_t inlen, char *out);


// ComputeIterativeSha256Hex:
//   Convenience function that performs the assignment kernel:
//     H0 = SHA256(seed)
//     For i = 1..iterations:
//       Hi = SHA256( Hi-1 || 0x01 )
//   Writes 64 hex characters + null into out_hex (caller must provide 65 bytes).
// 
//   semantics:
//     seed      - pointer to seed bytes
//     seed_len  - number of bytes in seed
//     iterations - number of iterations to perform (0 means return Hex(H0))
//     out_hex   - buffer of at least 65 bytes (64 hex chars + '\0')
// 
void ComputeIterativeSha256Hex(const uint8_t *seed, size_t seed_len,
                              uint32_t iterations, char out_hex[65]);


// ComputeSha256:
//   Compute SHA-256 of `data` (length `len` bytes).
//   Writes exactly 32 bytes to `out` (caller provides a buffer of at least 32 bytes).
// 
void ComputeSha256(const uint8_t *data, size_t len, uint8_t out[32]);


#ifdef __cplusplus
}
#endif

#endif  // PROJ1_INCLUDE_SHA256_H
