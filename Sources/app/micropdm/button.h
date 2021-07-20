/**
 * Keypad-related defines.
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <core/timer.h>
#include <core/pt.h>

#define MAX_BUTTONS					12

// FSD is ~11V, so treat > 5V as "on".
#define KL15_THRESHOLD				30000U

/**
 * Button config.
 */
typedef struct {
	uint8_t		ignition:1;
#define PARAM_IGNITION				0

	uint8_t		latching:1;
#define PARAM_LATCHING				1

	uint8_t		group:2;
#define PARAM_GROUP					2
#define GROUP_NONE					0	

	uint8_t		behaviour:4;
#define PARAM_BEHAVIOUR				3
#define BEHAVIOUR_NONE				0
#define BEHAVIOUR_FLASH_4HZ			1
#define BEHAVIOUR_FLASH_1_5HZ		2
#define BEHAVIOUR_FLASH_1MIN		3
#define BEHAVIOUR_DELAY_ON_500MS	4
#define BEHAVIOUR_AUTO_OFF_5S		5
#define BEHAVIOUR_AUTO_OFF_2MIN		6
#define BEHAVIOUR_AUTO_OFF_10MIN	7
#define BEHAVIOUR_AUTO_OFF_30MIN	8
#define BEHAVIOUR_IGNITION_MODE		9
#define BEHAVIOUR_NIGHT_MODE		10
#define BEHAVIOUR_MAX				11
	
	uint8_t		output:4;		
#define PARAM_OUTPUT				4
#define OUTPUT_NONE					0

	uint8_t		on_color:4;
#define PARAM_ON_COLOR				5
#define COLOR_NONE					0

	uint8_t		off_color:3;
#define PARAM_OFF_COLOR				6

	uint8_t		external_control:4;
#define PARAM_EXTERNAL_CONTROL		7
#define EXTERNAL_CONTROL_NONE		0
	
	uint8_t		res1:1;

	uint16_t	delayed_shutdown:3;
#define PARAM_DELAYED_SHUTDOWN		8
#define DELAYED_SHUTDOWN_NONE		0

	uint16_t	spare:5;
#define PARAM_MAX					9
} button_param_t;
	

/**
 * Button state.
 */
typedef struct {
	uint16_t		behaviour_timer;	// 1 tick = ~32ms
	uint16_t		shutdown_timer;		// 1 tick = 1s
	button_param_t	param;
	uint8_t			state;
} button_state_t;

extern void			button_init(void);
extern void			button_loop(void);
extern void			button_set_state(uint8_t button, bool new_state);
extern bool			button_get_state(uint8_t button);

#endif /* BUTTON_H_ */
