/*
 * Utility library.
 */

#ifndef LIB_H_
#define LIB_H_

#include <stdio.h>
#include <stdtypes.h>

typedef unsigned char   uint8_t;
typedef unsigned int    uint16_t;
typedef unsigned long   uint32_t;

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

#endif /* LIB_H_ */
