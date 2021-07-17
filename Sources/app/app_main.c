/*
 * Application top-level logic.
 */

#include <core/io.h>
#include <core/lib.h>
#include <core/pt.h>

//#include <app/applets/blink_keypad_test.h>
#include <app/applets/hsd_calibration.h>

/**
 * Called once at startup.
 */
void
app_init()
{
#ifdef APPLET_INIT
	APPLET_INIT();
#endif
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
#ifdef APPLET_LOOP
	APPLET_LOOP();
#endif
}
