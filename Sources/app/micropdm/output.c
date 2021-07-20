/**
 * Local high/low-side driver outputs.
 */

#include <core/io.h>
#include <core/mrs_bootrom.h>

#include <app/micropdm/output.h>
#include <app/micropdm/pdm_can.h>

static uint8_t remote_state;
static uint8_t local_state;

static void output_update(void);

void
output_init(void)
{
	(void)PWM_1_Disable();
	(void)PWM_1_ClrValue();
	(void)PWM_2_Disable();
	(void)PWM_2_ClrValue();
	(void)PWM_3_Disable();
	(void)PWM_3_ClrValue();
	(void)PWM_4_Disable();
	(void)PWM_4_ClrValue();
	if ((mrs_module_type == 'H') || (mrs_module_type == 'L')) {	
		(void)PWM_5_Disable();
		(void)PWM_5_ClrValue();
		(void)PWM_6_Disable();
		(void)PWM_6_ClrValue();
		(void)PWM_7_Disable();
		(void)PWM_7_ClrValue();
	}
	if (mrs_module_type == 'X') {
		DO_HSD_SEN_ClrVal();
	}
	if (mrs_module_type == 'H') {
		DO_HSD_SEN1_ClrVal();
		DO_HSD_SEN2_ClrVal();
	}
}

void
output_set_local(uint8_t output, bool state)
{
	if (state) {
		local_state |= (1 << output);
	} else {
		local_state &= !(1 << output);
	}
	output_update();
}

void
output_set_remote(uint8_t output, bool state)
{
	if (state) {
		remote_state |= (1 << output);
	} else {
		remote_state &= ~(1 << output);
	}
	output_update();
}

static void
output_update(void)
{
	uint8_t	state = local_state | remote_state;
	
	if (state & (1 << 0)) {
		(void)PWM_1_SetValue();		
	} else {
		(void)PWM_1_ClrValue();
	}
	if (state & (1 << 1)) {
		(void)PWM_2_SetValue();		
	} else {
		(void)PWM_2_ClrValue();
	}
	if (state & (1 << 2)) {
		(void)PWM_3_SetValue();		
	} else {
		(void)PWM_3_ClrValue();
	}
	if (state & (1 << 3)) {
		(void)PWM_4_SetValue();		
	} else {
		(void)PWM_4_ClrValue();
	}
	if ((mrs_module_type == 'H') || (mrs_module_type == 'L')) {	
		if (state & (1 << 4)) {
			(void)PWM_5_SetValue();		
		} else {
			(void)PWM_5_ClrValue();
		}
		if (state & (1 << 5)) {
			(void)PWM_6_SetValue();		
		} else {
			(void)PWM_6_ClrValue();
		}
		if (state & (1 << 6)) {
			(void)PWM_7_SetValue();		
		} else {
			(void)PWM_7_ClrValue();
		}
	}
}

uint8_t
output_get_local(void)
{
	return local_state;
}

uint8_t
output_get_remote(void)
{
	return remote_state;
}
