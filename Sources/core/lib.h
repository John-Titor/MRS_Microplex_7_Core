/*
 * Utility library.
 */

#ifndef CORE_LIB_H_
#define CORE_LIB_H_

#include <PE_Types.h>

/**
 * Assert that a condition is true.
 */
#define REQUIRE(_cond)                                          \
        do {                                                    \
            if (!(_cond))  __require_abort(__FILE__, __LINE__); \
        } while(0)

#define ABORT()     REQUIRE(0)

extern void __require_abort(const char *file, int line);

/**
 * Wrapper for printf().
 *
 * Automatically adds a newline, and sends output via CAN.
 */
extern void print(const char *format, ...);

/**
 * Wrapper for printf().
 *
 * Sends output via CAN.
 */
extern void printn(const char *format, ...);

/**
 * Print a hexdump of a range of memory.
 */
extern void hexdump(uint8_t *addr, unsigned int count);

#endif /* LIB_H_ */
