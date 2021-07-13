/*
 * Application-level callbacks.
 */

#include "can.h"
#include "io.h"

bool
app_can_filter(can_buf_t *buf)
{
    // by default, queue every message
    return TRUE;
}

void
app_can_receive(can_buf_t *buf)
{
}

void
app_can_idle(bool is_idle)
{
}
