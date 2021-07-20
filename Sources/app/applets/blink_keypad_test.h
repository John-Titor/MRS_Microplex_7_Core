/*
 * blink_keypad_test.c
 *
 *  Created on: Jul 15, 2021
 *      Author: DrZip
 */

#include <can_devices/blink_keypad.h>

#define APPLET_INIT			blink_keypad_test_init
#define APPLET_LOOP			blink_keypad_test_loop
#define APPLET_CAN_RECEIVE	blink_keypad_test_can_receive

static void
blink_keypad_test_init()
{
	bk_init();
	bk_set_backlight_color(BK_BL_COLOR_MAGENTA);
	bk_set_backlight_intensity(0x20);
}

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
            bk_set_key_led(key, BK_KEY_COLOR_OFF, 0);
            break;
        case BK_EVENT_SHORT_PRESS:
            print("key %d short press", key);
            bk_set_key_led(key, BK_KEY_COLOR_GREEN, 0);
            break;
        case BK_EVENT_LONG_PRESS_1:
            print("key %d long press 1", key);
            bk_set_key_led(key, BK_KEY_COLOR_BLUE, 0xaa);
            break;
        case BK_EVENT_LONG_PRESS_2:
            print("key %d long press 2", key);
            bk_set_key_led(key, BK_KEY_COLOR_CYAN, 0xaa);
            break;
        case BK_EVENT_LONG_PRESS_3:
            print("key %d long press 3", key);
            bk_set_key_led(key, BK_KEY_COLOR_MAGENTA, 0xaa);
            break;
        }
    }
    
    bk_loop();
}

static void
blink_keypad_test_can_receive(can_buf_t *buf)
{
	(void)bk_can_receive(buf);
}
