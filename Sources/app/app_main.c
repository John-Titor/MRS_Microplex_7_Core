/*
 * Application top-level logic.
 */

#include <core/io.h>
#include <core/lib.h>
#include <core/pt.h>

#include <can_devices/blink_keypad.h>

/**
 * Called once at startup.
 */
void
app_init()
{
    print("app init");
    bk_set_key_led(0, BK_RED, 0);
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
    uint8_t evt = bk_get_event();
    
    if (evt != BK_EVENT_NONE) {
        print("event %d", evt);
    }
}
