/*
 * button.c
 *
 *  Created on: Jul 18, 2021
 *      Author: DrZip
 */

#include <IEE1.h>
#include <AD1.h>

#include <core/io.h>
#include <core/pt.h>

#include <can_devices/blink_keypad.h>

#include <app/micropdm/button.h>

/**  
 * EEPROM config structure
 *
 * XXX TODO: two copies of the button config with a ping-pong
 *           scheme to avoid corruption due to power loss while saving.
 */
const struct {
	uint8_t			version;
	uint8_t			param[MAX_BUTTONS][PARAM_MAX];
	uint8_t			sum;	// sum of all bytes including this one = 0
} ee_config@(IEE1_AREA_START + 0x200);

/**
 * Current button state.
 */
static button_state_t	state[MAX_BUTTONS];

/**
 * Current operational mode.
 */
static uint8_t			system_mode;
#define MODE_STANDBY		0
#define MODE_CONFIG			1
#define MODE_OPERATION		2
#define MODE_SHUTDOWN_DELAY	3

/**
 * Table of shutdown delay times corresponding to config.delayed_shutdown,
 * in seconds.
 */
static const uint16_t shutdown_delays[] = {
	0,
	5,
	10,
	30,
	(1 * 60),
	(5 * 60),
	(10 * 60),
	(15 * 60)
};

/**
 * (Mostly) standard table of number -> color codes.
 * 
 * Configured LED color codes are special; they are offset by 1 with
 * 0 being "off".
 * 
 * Use these with a 50/50 duty cycle to get blinking behaviour.
 */
static const uint8_t color_codes[] = {
	BK_RED		| (BK_RED		<< 4),
	BK_GREEN	| (BK_GREEN		<< 4),
	BK_BLUE		| (BK_BLUE		<< 4),
	BK_YELLOW	| (BK_YELLOW	<< 4),
	BK_CYAN		| (BK_CYAN		<< 4),
	BK_MAGENTA	| (BK_MAGENTA	<< 4),
	BK_WHITE	| (BK_WHITE		<< 4),
	BK_RED		| (BK_OFF		<< 4),
	BK_GREEN	| (BK_OFF		<< 4),
	BK_BLUE		| (BK_OFF		<< 4),
	BK_YELLOW	| (BK_OFF		<< 4),
	BK_CYAN		| (BK_OFF		<< 4),
	BK_MAGENTA	| (BK_OFF		<< 4),
	BK_WHITE	| (BK_OFF		<< 4),
	BK_RED		| (BK_BLUE		<< 4)
};

static uint8_t	button_get_param_limit(uint8_t param);
static uint8_t	button_get_param(uint8_t button, uint8_t param);
static void		button_set_param(uint8_t button, uint8_t param, uint8_t value);
static void		button_set_defaults(void);
static bool		button_load_state(void);
static void		button_save_state(void);
static bool		button_check_KL15(void);
static void		button_set_output(uint8_t output, bool state);
static void		button_behaviour_timeout(uint8_t button);
static void		button_set_state(uint8_t button, bool new_state);
static void		button_enter_mode(uint8_t mode);
static void		button_standby_loop(void);
static void		button_config_loop(void);
static void		button_operation_loop(void);
static void		button_shutdown_delay_loop(void);

static uint8_t
button_get_param_limit(uint8_t param)
{
	static const uint8_t param_limits[] = {
			2,		// ignition
			2,		// latching
			4,		// group
			16,		// behaviour
			16,		// output
			16,		// on_color
			8,		// off_color
			16,		// ext_control
			8		// delay_shutdown
	};
	if (param < sizeof(param_limits)) {
		return param_limits[param];
	}
	return 1;		// force value to 0
}

static uint8_t
button_get_param(uint8_t button, uint8_t param)
{
	switch (param) {
	case PARAM_IGNITION:
		return state[button].param.ignition;
	case PARAM_LATCHING:
		return state[button].param.latching;
	case PARAM_GROUP:
		return state[button].param.group;
	case PARAM_BEHAVIOUR:
		return state[button].param.behaviour;
	case PARAM_OUTPUT:
		return state[button].param.output;
	case PARAM_ON_COLOR:
		return state[button].param.on_color;
	case PARAM_OFF_COLOR:
		return state[button].param.off_color;
	case PARAM_EXTERNAL_CONTROL:
		return state[button].param.external_control;
	case PARAM_DELAYED_SHUTDOWN:
		return state[button].param.delayed_shutdown;
	default:
		return 0;
	}
}

static void
button_set_param(uint8_t button, uint8_t param, uint8_t value)
{
	if (value < button_get_param_limit(param)) {
		switch (param) {
		case PARAM_IGNITION:
			state[button].param.ignition = value;
			break;
		case PARAM_LATCHING:
			state[button].param.latching = value;
			break;
		case PARAM_GROUP:
			state[button].param.group = value;
			break;
		case PARAM_BEHAVIOUR:
			state[button].param.behaviour = value;
			break;
		case PARAM_OUTPUT:
			state[button].param.output = value;
			break;
		case PARAM_ON_COLOR:
			state[button].param.on_color = value;
			break;
		case PARAM_OFF_COLOR:
			state[button].param.off_color = value;
			break;
		case PARAM_EXTERNAL_CONTROL:
			state[button].param.external_control = value;
			break;
		case PARAM_DELAYED_SHUTDOWN:
			state[button].param.delayed_shutdown = value;
			break;
		default:
			break;
		}
	}
}

/**
 * Set factory defaults.
 */
static void
button_set_defaults(void)
{
	uint8_t		i, j;
	
	for (i = 0; i < MAX_BUTTONS; i++) {
		static const uint8_t default_param[PARAM_MAX] = {
				0,						// ignition 
				0,						// latching
				GROUP_NONE,				// group
				BEHAVIOUR_NONE,			// behaviour
				OUTPUT_NONE,			// output
				COLOR_NONE,				// on_color
				COLOR_NONE,				// off_color
				EXTERNAL_CONTROL_NONE,	// external_control
				DELAYED_SHUTDOWN_NONE	// shutdown_delay
		};
		for (j = 0; j < PARAM_MAX; j++) {
			button_set_param(i, j, default_param[j]);
		}
	}
}

/** 
 * Load config from EEPROM
 */
static bool
button_load_state(void)
{
	uint8_t *p = (uint8_t *)&ee_config;
	uint8_t sum = 0;
	uint8_t i, j;

	// validate checksum
	while (p < ((uint8_t *)&ee_config + sizeof(ee_config))) {
		sum += *p++;
	}
	
	// if sum is valid, load config
	if (sum == 0) {
		for (i = 0; i < MAX_BUTTONS; i++) {
			for (j = 0; j < PARAM_MAX; j++) {
				button_set_param(i, j, ee_config.param[i][j]);
			}
		}
		return TRUE;
	} else {
		button_set_defaults();
		return FALSE;
	}
}

/**
 * Save config to EEPROM
 */
static void
button_save_state(void)
{
	uint8_t i, j;
	uint8_t sum;

	// set version (0)
	(void)IEE1_SetByte((uint16_t)&ee_config.version, 0);
	sum = 0;

	// copy config back to EEPROM
	for (i = 0; i < MAX_BUTTONS; i++) {
		for (j = 0; j < PARAM_MAX	; j++) {
			uint16_t ee_address = (uint16_t)&ee_config.param[i][j];
			uint8_t val = button_get_param(i, j);
			sum += val;
			(void)IEE1_SetByte(ee_address++, val);
		}
	}
	
	// store checksum byte last
	(void)IEE1_SetByte((uint16_t)&ee_config.sum, 0 - sum);
}

static bool
button_check_KL15(void)
{
	uint16_t value;
	
	(void)AD1_GetChanValue16(AI_KL15, &value);
	return value >= KL15_THRESHOLD;
}

static void
button_set_output(uint8_t output, bool state)
{
	// set output to state
	(void)output;
	(void)state;
}

/**
 * Handle behaviour-specific periodic activity.
 */
static void
button_behaviour_timeout(uint8_t button)
{
	bool	target_output_state = FALSE;
	uint8_t behaviour = button_get_param(button, PARAM_BEHAVIOUR); 
			
	switch (behaviour) {
	case BEHAVIOUR_NONE:
	case BEHAVIOUR_IGNITION_MODE:
		switch(behaviour) {
		case BEHAVIOUR_IGNITION_MODE:
			if (state[button].state == 0) {
				button_enter_mode(MODE_SHUTDOWN_DELAY);
			}
			break;
		}
		target_output_state = (state[button].state == 0) ? FALSE: TRUE;
		
		break;

	case BEHAVIOUR_FLASH_4HZ:
	case BEHAVIOUR_FLASH_1_5HZ:
	case BEHAVIOUR_FLASH_1MIN:
		switch (behaviour) {
			case BEHAVIOUR_FLASH_4HZ:
				state[button].behaviour_timer = 4;		// 4*32 = 128ms
				break;
			case BEHAVIOUR_FLASH_1_5HZ:
				state[button].behaviour_timer = 24;		// 24*32 = 768ms
				break;
			case BEHAVIOUR_FLASH_1MIN:
				state[button].behaviour_timer = 938;	// 938*32 ~= 30s
				break;
		}
		state[button].state = (state[button].state == 1) ? 2 : 1;
		target_output_state = (state[button].state == 1) ? FALSE : TRUE;
		break;
	
	case BEHAVIOUR_AUTO_OFF_5S:
	case BEHAVIOUR_AUTO_OFF_2MIN:
	case BEHAVIOUR_AUTO_OFF_10MIN:
	case BEHAVIOUR_AUTO_OFF_30MIN:
		// turn on?
		if (state[button].state == 1) {
			switch (behaviour) {
			case BEHAVIOUR_AUTO_OFF_5S:
				state[button].behaviour_timer = 156;
				break;
			case BEHAVIOUR_AUTO_OFF_2MIN:
				state[button].behaviour_timer = 3750;
				break;
			case BEHAVIOUR_AUTO_OFF_10MIN:
				state[button].behaviour_timer = 18750;
				break;
			case BEHAVIOUR_AUTO_OFF_30MIN:
				state[button].behaviour_timer = 56250;
				break;
			}
			state[button].state = 2;
			target_output_state = TRUE;
		} 

		// timer expired, turn off
		else if (state[button].state == 2){
			button_set_state(button, 0);
		}

		// explicit off request
		else {
			target_output_state = FALSE;
		}
		break;
		
	case BEHAVIOUR_NIGHT_MODE:
		if (state[button].state == 1) {
			bk_set_key_intensity(0x18);
			bk_set_backlight_intensity(0x10);
		} else {
			bk_set_key_intensity(0x3f);
			bk_set_backlight_intensity(0x00);
		}
		break;
	}
	
	button_set_output(button_get_param(button, PARAM_OUTPUT), target_output_state);
}

/**
 * Set a new state for a button.
 */
static void
button_set_state(uint8_t button, bool new_state)
{
	uint8_t	color;
	
	// button state is initially 0 / 1, may change due to behaviour
	state[button].state = new_state ? 1 : 0;

	// update button LED - in non-operation mode when turning off
	// buttons are turned off rather than going to their 'off' color.
	if (state[button].state) {
		uint8_t on_color = button_get_param(button, PARAM_ON_COLOR);
		if (on_color == 0) {
			color = BK_OFF;
		} else {
			color = color_codes[on_color - 1];
		}
	} else {
		uint8_t off_color = button_get_param(button, PARAM_OFF_COLOR);
		if ((system_mode != MODE_OPERATION)
				|| (off_color == 0)) {
			color = BK_OFF;
		} else {
			color = color_codes[off_color - 1];
		}
		state[button].behaviour_timer = 0;
		state[button].shutdown_timer = 0;
	}
	bk_set_key_led(button, color, 0x33);

	// trigger the behaviour timeout to set the output
	button_behaviour_timeout(button);
}

static void
button_enter_mode(uint8_t mode)
{
	uint8_t		i;

	// set new mode first
	system_mode = mode;
	
	// now perform mode-entry actions
	switch (system_mode) {
	case MODE_STANDBY:
		print("standby");
		// blink white LED for ignition-mode switches
		for (i = 0; i < MAX_BUTTONS; i++) {
			if (button_get_param(i, PARAM_BEHAVIOUR) == BEHAVIOUR_IGNITION_MODE) {
				// not quite the same as the original, would need to ramp brightness
				// in the standby mode loop to get it to breathe
				bk_set_key_led(i, BK_WHITE, 0x05);
			}
		}
		break;

	case MODE_CONFIG:
		print("config");
		bk_set_key_led(0, BK_WHITE, 0);
		for (i = 1; i < MAX_BUTTONS; i++) {
			bk_set_key_led(i, BK_GREEN, 0);
		}
		break;

	case MODE_OPERATION:
		print("operation");
		// Set button states; turns on ignition-triggered outputs
		// and makes all LED states sane
		for (i = 0; i < MAX_BUTTONS; i++) {
			button_set_state(i, button_get_param(i, PARAM_IGNITION));
		}
		break;
		
	case MODE_SHUTDOWN_DELAY:
		print("shutdown");
		// set shutdown timers
		for (i = 0; i < MAX_BUTTONS; i++) {
			
			// look up delay mode, compute timer value
			uint8_t delay_code = button_get_param(i, PARAM_DELAYED_SHUTDOWN);
			if (state[i].state && (delay_code != DELAYED_SHUTDOWN_NONE)) {
				// add 1 because timer is always expired when standby loop starts
				state[i].shutdown_timer = shutdown_delays[delay_code] + 1;
			} else {
				state[i].shutdown_timer = 0;
			}
			
			// turn off any output that doesn't have a nonzero timer value
			if (state[i].shutdown_timer == 0) {
				button_set_state(i, FALSE);
			}
		}
		break;
	}
}

/**
 * Standby-mode loop.
 */
void
button_standby_loop(void)
{
	static bool 	chord_detected;
	uint8_t			i;
	
	// Is KL15 off? Maybe go to configuration mode depending on
	// button chord.
	if (!button_check_KL15()) {
		
		// buttons 1 & 2 & 3 in at least long-press 2 mode?
		if ((bk_get_key_event(0) >= BK_EVENT_LONG_PRESS_2)
			&& (bk_get_key_event(1) >= BK_EVENT_LONG_PRESS_2)
			&& (bk_get_key_event(2) >= BK_EVENT_LONG_PRESS_2)) {
		
			// yes, set all key LEDs to green
			chord_detected = TRUE;
			for (i = 0; i < MAX_BUTTONS; i++) {
				bk_set_key_led(i, BK_GREEN, 0);
			}
		}
		
		// buttons all released after long-press detected?
		if (chord_detected
			&& (bk_get_key_event(0) == BK_EVENT_RELEASE)
			&& (bk_get_key_event(1) == BK_EVENT_RELEASE)
			&& (bk_get_key_event(2) == BK_EVENT_RELEASE)) {

			// yes, turn off all LEDs and go to config mode
			chord_detected = FALSE;
			for (i = 0; i < MAX_BUTTONS; i++) {
				bk_set_key_led(i, BK_OFF, 0);
			}
			button_enter_mode(MODE_CONFIG);
		}
	}

	// KL15 is on - maybe go to operation mode depending on button
	// requirements.
	else {
		bool go_to_operation = TRUE;
		uint8_t i;
		
		chord_detected = FALSE;		// reset the config-entry chord detector

		// check for a button set to ignition mode
		for (i = 0; i < MAX_BUTTONS; i++) {
			if (button_get_param(i, PARAM_BEHAVIOUR) == BEHAVIOUR_IGNITION_MODE) {
				uint8_t event_code = bk_get_event();
				
				// only go to operation mode if we see a press on the
				// ignition button
				// XXX should this be a long press?
				if (event_code != (BK_EVENT_SHORT_PRESS | i)) {
					go_to_operation = FALSE;
				} else {
					// update button state
					button_set_state(i, TRUE);
				}
			}
		}
		if (go_to_operation) {
			button_enter_mode(MODE_OPERATION);
		}
	}
}

/**
 * Configuration mode loop.
 */
static void
button_config_loop(void)
{
	static uint8_t	current_button;
	static uint8_t	current_param;
	static uint8_t	current_param_value;
	static uint8_t	mode;
#define CONFIG_MODE_BUTTON		0
#define CONFIG_MODE_PARAMETER	1
#define CONFIG_MODE_VALUE		2
#define CONFIG_MODE_MAX			3
	
	uint8_t			event_code = bk_get_event();
	uint8_t			i;
	
	// do nothing if there's no keypad input
	if (event_code == BK_EVENT_NONE) {
		return;
	}
	
	// Check for KL15 or the Exit button.
	//
	// Note that we can't check for a press on button 0 here
	// as a long-press is a request to save.
	//
	if ((button_check_KL15())
			|| (event_code == (BK_EVENT_RELEASE | 0))) {

		print("exit");

		// reset state for next time
		current_button = 0;
		current_param = 0;
		mode = CONFIG_MODE_BUTTON;
		
		// turn off LEDs
		for (i = 0; i < MAX_BUTTONS; i++) {
			bk_set_key_led(i, BK_OFF, 0);
		}
		
		button_enter_mode(MODE_STANDBY);
		return;
	}
	
	// handle other button presses
	switch (event_code) {

	// parameter save request - LEDs go red after save and will go
	// off when the release event is registered.
	case BK_EVENT_LONG_PRESS_2 | 0:
		button_save_state();
		for (i = 0; i < MAX_BUTTONS; i++) {
			bk_set_key_led(i, BK_RED, 0);
		}
		break;
	
	case BK_EVENT_LONG_PRESS_3 | 0:
		button_set_defaults();
		button_save_state();
		for (i = 0; i < MAX_BUTTONS; i++) {
			bk_set_key_led(i, BK_CYAN, 0);
		}
		break;
	
	// next value for current button / mode?
	case BK_EVENT_SHORT_PRESS | 1:
		print("next");
		switch (mode) {
		case CONFIG_MODE_BUTTON:
			if (++current_button >= bk_num_keys()) {
				current_button = 0;
			}
			break;
		case CONFIG_MODE_PARAMETER:
			if (++current_param >= PARAM_MAX) {
				current_param = 0;
			}
			break;
		case CONFIG_MODE_VALUE:
			if (++current_param_value >= button_get_param_limit(current_param)) {
				current_param_value = 0;
			}
			break;
		}
		break;
	
	// confirm and next mode?
	case BK_EVENT_SHORT_PRESS | 2:
		print("confirm");
		switch (mode) {

		// switch from button mode to parameter mode
		case CONFIG_MODE_BUTTON:
			mode = CONFIG_MODE_PARAMETER;
			current_param = 0;
			for (i = 1; i < MAX_BUTTONS; i++) {
				bk_set_key_led(i, BK_BLUE, 0);
			}
			break;

		// switch from parameter mode to value mode 
		case CONFIG_MODE_PARAMETER:
			mode = CONFIG_MODE_VALUE;
			current_param_value = button_get_param(current_button, current_param);
			for (i = 1; i < MAX_BUTTONS; i++) {
				bk_set_key_led(i, BK_MAGENTA, 0);
			}
			break;

		// confirm / store current parameter value and switch to button mode
		case CONFIG_MODE_VALUE:
			button_set_param(current_button, current_param, current_param_value);
			mode = CONFIG_MODE_BUTTON;
			break;
		}
		break;
	}
	
	// update LED state
	switch (mode) {
	case CONFIG_MODE_BUTTON:
		for (i = 0; i < MAX_BUTTONS; i++) {
			if (i == current_button) {
				bk_set_key_led(i, BK_WHITE, 0);
			} else {
				bk_set_key_led(i, BK_GREEN, 0);
			}
		}
		break;

	case CONFIG_MODE_PARAMETER:
		bk_set_key_led(0, color_codes[current_param], 0x55);
		break;

	case CONFIG_MODE_VALUE:
		switch (current_param) {
		// special handling for on/off color encoding
		case PARAM_ON_COLOR:
		case PARAM_OFF_COLOR:
			if (current_param_value == 0) {
				bk_set_key_led(0, BK_OFF, 0);
			} else {
				bk_set_key_led(0, color_codes[current_param_value - 1], 0x55);
			}
			break;
		default:
			bk_set_key_led(0, color_codes[current_param_value], 0x55);
			break;
		}
		break;		
	}
}

static void
button_operation_loop(void)
{
	static timer_t behaviour_tick;
	uint8_t event_code = bk_get_event();
	uint8_t event = event_code & BK_EVENT_MASK;
	uint8_t button = event_code & BK_KEY_MASK;
	bool target_state;
	uint8_t i;

	timer_register(behaviour_tick);
	
	switch (event) {
	case BK_EVENT_SHORT_PRESS:
	case BK_EVENT_LONG_PRESS_1:
		// latching buttons toggle on press
		if (state[button].param.latching) {
			target_state = state[button].state ? FALSE : TRUE;
		} 
		
		// non-latching (momentary) buttons set on press
		else {
			target_state = TRUE;
		}
		
		// handle delay-on behaviour
		if (target_state == TRUE) {
			if (button_get_param(button, PARAM_BEHAVIOUR) == BEHAVIOUR_DELAY_ON_500MS) {
				if (event != BK_EVENT_LONG_PRESS_1) {
					// need long-press to activate
					break;
				}
			} else {
				if (event != BK_EVENT_SHORT_PRESS) {
					// need short-press to activate
					break;
				}
			}
		}
		
		// button state change
		button_set_state(button, target_state);
		break;

	case BK_EVENT_RELEASE:
		// non-latching (momentary) buttons clear on press
		if (!state[button].param.latching) {
			button_set_state(button, 0);
		}
		break;
	}
	
	// Handle behaviour timers
	//
	if (timer_expired(behaviour_tick)) {
		timer_reset(behaviour_tick, 32);
		for (i = 0; i < MAX_BUTTONS; i++) {
			if (state[i].state 
				&&(state[i].behaviour_timer > 0)) {
				
				if (--state[i].behaviour_timer == 0) {
					button_behaviour_timeout(button);
				}
			}
		}
	}

}

/**
 * Shutdown-delay loop.
 */
void
button_shutdown_delay_loop(void)
{
	static timer_t	second_tick;
	uint8_t			i;
	bool			hang_in_mode = FALSE;
	uint8_t 		event_code = bk_get_event();
	uint8_t 		event = event_code & BK_EVENT_MASK;
	uint8_t 		button = event_code & BK_KEY_MASK;
	
	timer_register(second_tick);

	// handle delayed shutdown timers
	if (timer_expired(second_tick)) {
		timer_reset(second_tick, 1000);
		for (i = 0; i < MAX_BUTTONS; i++) {
			if (state[i].shutdown_timer > 0) {
				if (--state[i].shutdown_timer == 0) {
					button_set_state(i, FALSE);
				} else {
					hang_in_mode = TRUE;
				}
			}
		}
	}
	
	// handle manual-off button presses
	if (state[button].state != 0) {
		button_set_state(button, FALSE);
	}
	
	if (!hang_in_mode) {
		button_enter_mode(MODE_STANDBY);
	}
}

/**
 * Initialize the button module.
 */
void
button_init(void)
{
	if (!button_load_state()) {
		// EEPROM load failed, fast-blink key 0 red 
		bk_set_key_led(0, BK_RED, 0x55);
		// XXX delay here? watchdog...
	}
	system_mode = MODE_STANDBY;
}

/**
 * Main button loop.
 */
void
button_loop(void)
{
	static bool			t15_state;

	// Force entry to shutdown mode on loss of KL15.
	//
	if (!button_check_KL15()) {
		// was ignition on?
		if (t15_state) {
			button_enter_mode(MODE_SHUTDOWN_DELAY);
		}
		t15_state = FALSE;
	}
	
	switch (system_mode) {
	default:
	case MODE_STANDBY:
		button_standby_loop();
		break;
	case MODE_CONFIG:
		button_config_loop();
		break;
	case MODE_OPERATION:
		button_operation_loop();
		break;
	case MODE_SHUTDOWN_DELAY:
		button_shutdown_delay_loop();
		break;
	}
}
