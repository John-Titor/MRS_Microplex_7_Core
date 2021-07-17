/*
 * Generic library-ish things.
 */

#include <WDog1.h>

#include <stdio.h>

#include <core/can.h>
#include <core/lib.h>

void
print(const char *format, ...)
{
    va_list args;

    set_printf(can_putchar);
    va_start(args, format);
    (void)vprintf(format, args);
    va_end(args);
    can_putchar('\n');
}

void
printn(const char *format, ...)
{
    va_list args;

    set_printf(can_putchar);
    va_start(args, format);
    (void)vprintf(format, args);
}

void
hexdump(uint8_t *addr, unsigned int count)
{
    while (count) {
        uint8_t i;
        
        printn("%p:", addr);
        for (i = 0; count && (i < 16); i++) {
            printn(" %02x", *addr++);
            count--;
        }
        print("");
        WDog1_Clear();
    }
}

void
__require_abort(const char *file, int line)
{
    print("ABORT: %s:%d", file, line);
    for (;;);
}

