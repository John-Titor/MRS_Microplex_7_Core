/*
 * Configuration parameters.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* 
 * Timeout (ms) before deciding that CAN has been disconnected.
 */
#define CAN_IDLE_TIMEOUT            2000

/*
 * Size of the CAN receive FIFO.
 */
#define CAN_RX_FIFO_SIZE			8

/*
 * Minimum load current (mA): below this, output is considered open.
 */
#define SENSE_OPEN_CURRENT          50

/*
 * Maximum load current (mA): over this, output is considered overloaded.
 */
#define SENSE_OVERLOAD_CURRENT      2500

/*
 * Maximum off voltage (mV): over this, output is considered stuck/shorted to +12.
 */
#define SENSE_STUCK_VOLTAGE         2000

/*
 * Inrush current settling time (ms)
 */
#define SENSE_INRUSH_DELAY          50

/*
 * Turn-off current settling time (ms)
 */
#define SENSE_SETTLE_DELAY          500

/*
 * Delay between retries for an overloaded output (ms).
 */
#define SENSE_OVERLOAD_RETRY_INTERVAL   1000

/*
 * Blink Marine keypad configuration 
 */
//#define BK_FIXED_KEYPAD_ID      0x15  // assume keypad ID
#define BK_MAX_KEYS             12      // largest keypad supported
#define BK_IDLE_TIMEOUT_MS      1000    // timeout before assuming keypad gone
#define BK_UPDATE_PERIOD_MS     25      // expected keypad update interval
#define BK_BLINK_PERIOD_MS      250     // time per pattern bit
#define BK_TICK_PERIOD_MS       25      // interval between ticks
#define BK_SHORT_PRESS_TICKS    2       // delay before registering a short press
#define BK_LONG_PRESS_1_TICKS   20      // delay before registering first long press
#define BK_LONG_PRESS_2_TICKS   60      // delay before registering a long press
#define BK_LONG_PRESS_3_TICKS  120      // delay before registering a long press

#endif // _CONFIG_H
