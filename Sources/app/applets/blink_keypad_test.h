/*
 * blink_keypad_test.c
 *
 *  Created on: Jul 15, 2021
 *      Author: DrZip
 */

#include <can_devices/blink_keypad.h>

#define APPLET_LOOP	blink_keypad_test_loop

static void 
blink_keypad_test_loop(void)
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
