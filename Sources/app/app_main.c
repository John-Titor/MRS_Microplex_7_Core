/*
 * Application top-level logic.
 */

#include <core/io.h>
#include <core/lib.h>
#include <core/pt.h>

/**
 * Called once at startup.
 */
void
app_init()
{
    print("app init");
}

/**
 * Called each time around the main loop. 
 * 
 * Blocking this loop will affect the functioning of other parts
 * of the system; blocking for long enough will eventually trip 
 * the watchdog.
 */
void
app_loop()
{
}
