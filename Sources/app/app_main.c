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
        uint8_t key = evt & BK_KEY_MASK;
        uint8_t event = evt & BK_EVENT_MASK;
        
        switch (event) {
        case BK_EVENT_RELEASE:
            print("key %d released", key);
            bk_set_key_led(key, BK_OFF, 0);
            break;
        case BK_EVENT_SHORT_PRESS:
            print("key %d short press", key);
            bk_set_key_led(key, BK_GREEN, 0);
            break;
        case BK_EVENT_LONG_PRESS:
            print("key %d long press", key);
            bk_set_key_led(key, BK_BLUE, 0xaa);
            break;
        }
    }
}
