/*
 * Application top-level logic.
 */

#include "io.h"
#include "lib.h"
#include "pt.h"

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
