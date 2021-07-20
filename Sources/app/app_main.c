/**
 * Application top-level logic.
 */

#include <core/callbacks.h>
#include <core/can.h>
#include <core/io.h>
#include <core/lib.h>
#include <core/pt.h>

#include <can_devices/blink_keypad.h>

//#include <app/applets/blink_keypad_test.h>
//#include <app/applets/hsd_calibration.h>
//#include <app/applets/interrupt_load_test.h>

#include <app/micropdm/button.h>
#include <app/micropdm/output.h>
#include <app/micropdm/pdm_can.h>

/**
 * Called once at startup.
 */
void
app_init()
{
#ifdef APPLET_INIT
	APPLET_INIT();
#else
	bk_init();
	button_init();
	output_init();
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
	bk_loop();
	button_loop();
	pdm_can_loop();
#endif
}

bool
app_can_filter(can_buf_t *buf)
{
#ifdef APPLET_CAN_FILTER
	return APPLET_CAN_FILTER(buf);
#endif
	return bk_can_filter(buf) || pdm_can_filter(buf);
}

void
app_can_receive(can_buf_t *buf)
{
#ifdef APPLET_CAN_RECEIVE
	APPLET_CAN_RECEIVE(buf);
#endif
	if (!bk_can_receive(buf)) {
		pdm_can_receive(buf);
	}
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
