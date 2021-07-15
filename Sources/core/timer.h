/*
 * Timers and timebase.
 */

#ifndef CORE_TIMER_H_
#define CORE_TIMER_H_

#include <Cpu.h>

#include <core/lib.h>

/**
 * Interrupt callback on 1ms timer tick.
 */
extern void timer_tick(void);

/**
 * One-shot timer.
 */
typedef struct _timer {
    struct _timer       *_next;
    volatile uint16_t   delay_ms;
} timer_t;

/**
 *  One-shot or periodic timer callback.
 */
typedef struct _timer_call {
    void                (*callback)(void);  // function to call - must be interrupt-safe
    volatile uint16_t   delay_ms;
    uint16_t            period_ms;          // tick interval between calls, 0 for one-shot
    struct _timer_call  *_next;
} timer_call_t;

/**
 * Register a one-shot timer.
 * 
 * @note does nothing to an already-registered timer.
 */
#define timer_register(_t)      _timer_register(&_t)
extern void                     _timer_register(timer_t *timer);

/**
 * Register a timer callback.
 * 
 * @note does nothing to an already-registered callback.
 */
#define timer_call_register(_t) _timer_call_register(&_t)
extern void                     _timer_call_register(timer_call_t *call);

/** 
 * Reset a one-shot timer or timer callback
 */
#define timer_reset(_timer, _delay)         \
        do {                                \
            EnterCritical();                \
            (_timer).delay_ms = _delay;     \
            ExitCritical();                 \
        } while(0)

/**
 * Test whether a timer or callback has expired
 */
#define timer_expired(_timer)           ((_timer).delay_ms == 0)

/**
 * Blocking delay for protothreads
 * 
 * The current thread will be blocked until the delay has expired.
 * 
 * @param pt            The current protothread
 * @param timer         The timer to use
 * @param ms            The number of milliseconds to block
 */
#define pt_delay(pt, timer, ms)                 \
        do {                                    \
            timer_reset(timer, ms);             \
            pt_wait(pt, timer_expired(timer));  \
        } while(0)

#endif // _TIMER_H
