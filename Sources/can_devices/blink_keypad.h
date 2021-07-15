/*
 * blink_keypad.h
 *
 * Driver for the Blink Marine keypads in CanOpen mode.
 * 
 */

#ifndef BLINK_KEYPAD_H_
#define BLINK_KEYPAD_H_

#include <core/can.h>
#include <core/lib.h>

/**
 * Keypad event codes.
 */
#define BK_EVENT_NONE           0xff
#define BK_EVENT_RELEASE        0x00
#define BK_EVENT_SHORT_PRESS    0x10
#define BK_EVENT_LONG_PRESS     0x20
#define BK_EVENT_MASK           0xf0
#define BK_KEY_MASK             0x0f

/**
 * Color codes.
 */
#define BK_OFF                  0x0
#define BK_RED                  0x1
#define BK_GREEN                0x2
#define BK_BLUE                 0x4
#define BK_YELLOW               (BK_RED | BK_GREEN)
#define BK_CYAN                 (BK_GREEN | BK_BLUE)
#define BK_MAGENTA              (BK_RED | BK_BLUE)
#define BK_WHITE                (BK_RED | BK_GREEN | BK_BLUE)
#define BK_COLOR_MASK           0xf

#define BK_MAX_INTENSITY    0x3f

/**
 * Initialize keypad support. 
 */
extern void bk_init(void);

/**
 * Call to run the keypad logic.
 */
extern void bk_loop(void);

/**
 * Feed a received CAN message to the keypad handler.
 * 
 * Should be called from app_can_receive for any message that
 * might be from a keypad.
 * 
 * @param buf           The CAN message buffer.
 * @returns             TRUE if the message was consumed, FALSE 
 *                      if it was not a keypad message.
 */
extern bool bk_can_receive(can_buf_t *buf);

/**
 * Get an event from the keypad.
 * 
 * @returns             Event code in the high 4 bits, key number in the low 4.
 *                      BK_EVENT_NONE if no event occurred.
 */
extern uint8_t bk_get_event(void);

/**
 * Get the most recent event for a given key.
 *
 * Note that this returns the event that put the key into its current
 * state; it will never return BK_EVENT_NONE.
 * 
 * @param key           Key number (0-11)
 * @returns             Event code in the high 4 bits.
 */
extern uint8_t bk_get_key_event(uint8_t key);

/**
 * Set a key LED.
 *
 * @param key           Key number (0-11).
 * @param colors        Colors A and B in the low and high 4 bits respectively.
 * @param pattern       Each bit corresponds to a 250ms slot in a 2s repeating cycle.
 *                      A zero bit gives color A, a 1 bit gives color b.  All-zeros
 *                      gives constant color A, etc.
 */
extern void bk_set_key_led(uint8_t key, uint8_t colors, uint8_t pattern);

/**
 * Set key LED brightness.
 * 
 * @param intensity     Key LED intensity (0-0x3f)
 */
extern void bk_set_key_intensity(uint8_t intensity);

/**
 * Set the backlight brightness.
 * 
 * @param intensity     Backlight intensity (0-0x3f).
 */
extern void bk_set_backlight_intensity(uint8_t intensity);

/**
 * Standalone keypad discover / configure routine.
 * 
 * Call from app_init() to discover and configure a single Blink Marine
 * keypad.  Not not suitable for use at regular startup.
 * 
 * Sets the keypad to:
 *  - CANOpen mode
 *  - Current bootrom CAN speed
 *  - auto-start
 *  - boot-message enabled
 *  - short startup flash
 *  - configured announce interval
 */
void bk_configure(void);


#endif /* BLINK_KEYPAD_H_ */
