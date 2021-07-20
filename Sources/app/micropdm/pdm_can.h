/*
 * can.h
 *
 *  Created on: Jul 18, 2021
 *      Author: DrZip
 */

#ifndef CAN_H_
#define CAN_H_

#include <core/can.h>

#define PDM_CMD_ID		(0x00322e12 | CAN_ID_EXT)
#define PDM_CTRL_ID		(0x00322e16 | CAN_ID_EXT)

extern bool pdm_can_filter(can_buf_t *buf);
extern void pdm_can_receive(can_buf_t *buf); 
extern void pdm_can_set_extender(uint8_t output, bool state);
extern void pdm_can_loop(void);

#endif /* CAN_H_ */
