/*
 * Application top-level logic.
 */

#include <core/callbacks.h>
#include <core/can.h>
#include <core/io.h>
#include <core/lib.h>
#include <core/pt.h>

#include <app/applets/blink_keypad_test.h>
//#include <app/applets/hsd_calibration.h>
//#include <app/applets/interrupt_load_test.h>

#include <app/micropdm/button.h>

/**
 * Called once at startup.
 */
void
app_init()
{
#ifdef APPLET_INIT
	APPLET_INIT();
#else
	button_init();
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
#else
	button_loop();
#endif
}

bool
app_can_filter(can_buf_t *buf)
{
#ifdef APPLET_CAN_FILTER
	return APPLET_CAN_FILTER(buf);
#endif
    // by default, queue every message
    (void)buf;
    return TRUE;
}

void
app_can_receive(can_buf_t *buf)
{
#ifdef APPLET_CAN_RECEIVE
	APPLET_CAN_RECEIVE(buf);
#endif
    (void)buf;
}

void
app_can_idle(bool is_idle)
{
    (void)is_idle;
}

void
app_adc_ready(void)
{
}
