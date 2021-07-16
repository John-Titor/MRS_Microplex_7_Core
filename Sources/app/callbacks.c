/**
 * Application-level callbacks.
 */

#include <core/callbacks.h>
#include <core/can.h>
#include <core/io.h>

bool
app_can_filter(can_buf_t *buf)
{
    // by default, queue every message
    (void)buf;
    return TRUE;
}

void
app_can_receive(can_buf_t *buf)
{
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
