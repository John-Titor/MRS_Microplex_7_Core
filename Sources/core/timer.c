/*
 * Timers and timed callbacks.
 */

#include <stddef.h>

#include <core/timer.h>

#define TIMER_LIST_END              (timer_t *)1
#define TIMER_CALL_LIST_END         (timer_call_t *)2

#define timer_registered(_timer)    ((_timer)._next != NULL)

static timer_t          *timer_list = TIMER_LIST_END;
static timer_call_t     *timer_call_list = TIMER_CALL_LIST_END;

void
_timer_register(timer_t *timer)
{
    EnterCritical();

    if (!timer_registered(*timer)) {
        // singly-linked insertion at head
        timer->_next = timer_list;
        timer_list = timer;
    }

    ExitCritical();
}

void
_timer_call_register(timer_call_t *call)
{
    EnterCritical();

    if (!timer_registered(*call)) {

        // singly-linked insertion at head
        call->_next = timer_call_list;
        timer_call_list = call;
    }   

    ExitCritical();
}

void
timer_tick(void)
{
    timer_t *t;
    timer_call_t *tc;

    // update timers
    for (t = timer_list; t != TIMER_LIST_END; t = t->_next) {
        if (t->delay_ms > 0) {
            t->delay_ms--;
        }
    }

    // run timer calls
    for (tc = timer_call_list; tc != TIMER_CALL_LIST_END; tc = tc->_next) {

        // if the call is active...
        if (tc->delay_ms > 0) {
            // and the delay expires...
            if (--tc->delay_ms == 0) {
                // run the callback
                tc->callback();
                // and reload the delay (or leave it at
                // zero for a one-shot)
                tc->delay_ms = tc->period_ms;
            }
        }
    }
}
