/**
 * Application-level callbacks.
 */

#include <core/callbacks.h>
#include <core/can.h>
#include <core/io.h>

#include <app/micropdm/pdm_can.h>

bool
app_can_filter(can_buf_t *buf)
{
	// native microPDM control messages
	if ((buf->id == PDM_CMD_ID)
		|| (buf->id == PDM_CTRL_ID)) {
		return TRUE;
	}
    return FALSE;
}

void
app_can_receive(can_buf_t *buf)
{
	pdm_can_receive(buf);
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
