/*
 * output.h
 *
 *  Created on: Jul 18, 2021
 *      Author: DrZip
 */

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <core/lib.h>

extern void 	output_init(void);
extern void		output_set_local(uint8_t output, bool state);
extern void		output_set_remote(uint8_t output, bool state);
extern uint8_t	output_get_local(void);
extern uint8_t	output_get_remote(void);

#endif /* OUTPUT_H_ */
