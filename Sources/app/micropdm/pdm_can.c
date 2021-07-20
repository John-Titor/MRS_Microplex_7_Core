/**
 * CAN message handling and generation.
 */

#include <AD1.h>

#include <core/can.h>
#include <core/io.h>
#include <core/mrs_bootrom.h>

#include <can_devices/blink_keypad.h>

#include <app/micropdm/button.h>
#include <app/micropdm/output.h>
#include <app/micropdm/pdm_can.h>

static uint8_t extender_state;

bool
pdm_can_filter(can_buf_t *buf)
{
	// native microPDM control messages
	if ((buf->id == PDM_CMD_ID)
		|| (buf->id == PDM_CTRL_ID)) {
		return TRUE;
	}
	return FALSE;
}

void
pdm_can_receive(can_buf_t *buf)
{
	// operation command
	if ((buf->id == PDM_CMD_ID) && (buf->dlc == 8)) {
	
		switch (buf->data[0]) {
		case 0x01:			// configure button
			// XXX
			break;
		case 0x02:			// activate button
			button_set_state(buf->data[1], buf->data[2]);
			break;
		case 0x03:			// change bus speed
			// XXX
			break;
		}
	}
	
	// output force operation; disregards button configuration
	else if ((buf->id == PDM_CTRL_ID) && (buf->dlc == 8)) {
		uint8_t i;
		
		for (i = 0; i < 7; i++) {
			output_set_remote(i, (buf->data[0] & (1 << i)));
		}
	}
}

static void
pdm_can_send_button_report(void)
{
	uint8_t data[8] = {0};
	uint8_t i;

	// encode button & commanded output states
	for (i = 0; i < MAX_BUTTONS; i++) {
		bool led_state = (bk_get_key_led(i) != BK_KEY_COLOR_OFF);
		bool button_state = button_get_state(i);
	
		if (led_state & button_state) {
			data[0 + (i / 8)] |= (1 << (i % 8));			
		}
		if (led_state) {
			data[2 + (i / 8)] |= (1 << (i % 8));			
		}
	}	
	data[4] = output_get_local();
	data[5] = 0;
	data[6] = 0;
	data[7] = 0;
	can_tx_async(0x322e13 | CAN_ID_EXT, 8, data);
}

static void
pdm_can_send_current_report(void)
{
	uint8_t data[8];
	uint16_t cval;
	
	(void)AD1_GetChanValue16(AI_CS_1, &cval);
	cval >>= 4;
	data[0] = cval & 0xff;
	data[1] = cval >> 8;

	(void)AD1_GetChanValue16(AI_CS_2, &cval);
	cval >>= 4;
	data[2] = cval & 0xff;
	data[3] = cval >> 8;

	(void)AD1_GetChanValue16(AI_CS_3, &cval);
	cval >>= 4;
	data[4] = cval & 0xff;
	data[5] = cval >> 8;

	(void)AD1_GetChanValue16(AI_CS_4, &cval);
	cval >>= 4;
	data[6] = cval & 0xff;
	data[7] = cval >> 8;
	
	can_tx_async(0x322e14 | CAN_ID_EXT, 8, data);

	if (mrs_module_type == 'H') {
		(void)AD1_GetChanValue16(AI_CS_5, &cval);
		cval >>= 4;
		data[0] = cval & 0xff;
		data[1] = cval >> 8;
	
		(void)AD1_GetChanValue16(AI_CS_6, &cval);
		cval >>= 4;
		data[2] = cval & 0xff;
		data[3] = cval >> 8;
	
		(void)AD1_GetChanValue16(AI_CS_7, &cval);
		cval >>= 4;
		data[4] = cval & 0xff;
		data[5] = cval >> 8;
		
		data[6] = data[7] = 0;
		
		can_tx_async(0x322e15 | CAN_ID_EXT, 8, data);
	}
}

static void
pdm_can_send_extender_update(void)
{
	uint8_t data[8] = { 0 };
	
	data[0] = extender_state;
	can_tx_async(0x322e26 | CAN_ID_EXT, 8, data);
	can_tx_async(0x322e27 | CAN_ID_EXT, 8, data);
}

void
pdm_can_loop(void)
{
	static timer_t	button_report;
	static timer_t	extender_update;
	
	timer_register(button_report);
	timer_register(extender_update);
	
	if (timer_expired(button_report)) {
		timer_reset(button_report, 25);
		pdm_can_send_button_report();		
	}
	
	if (timer_expired(extender_update)) {
		timer_reset(extender_update, 25);
		pdm_can_send_extender_update();
	}

	if ((mrs_module_type == 'X') || (mrs_module_type == 'H')) {
		static timer_t	current_report;
		timer_register(current_report);

		if (timer_expired(current_report)) {
			timer_reset(current_report, 50);
			pdm_can_send_current_report();
		}
	}
}

void
pdm_can_set_extender(uint8_t output, bool state)
{
	if (output < 7) {
		if (state) {
			extender_state |= (1 << output);
		} else {
			extender_state &= ~(1 << output);
		}
	}
}
