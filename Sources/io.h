/*
 * io.h
 *
 * Master pin I/O include; handles target-specific I/O
 * selection and remapping.
 */

#ifndef IO_H_
#define IO_H_

// Standard IOs common to every target
#include "DI_CAN_ERR.h"
#include "CAN_EN.h"
#include "CAN_STB_N.h"
#include "CAN_WAKE.h"
#include "DO_POWER.h"
#include "PWM_1.h"
#include "PWM_2.h"
#include "PWM_3.h"
#include "PWM_4.h"
#include "PWM_6.h"
#include "PWM_7.h"

#define AI_KL15		AD1_CHANNEL_KL15
#define AI_TEMP		AD1_CHANNEL_TEMP

#ifdef TARGET_7X
#include "DO_1.h"
#include "DO_2.h"
#include "DO_30V_10V_1.h"
#include "DO_30V_10V_2.h"
#include "DO_30V_10V_3.h"

// Map 7X I/O names to generic pin names
#define AI_3_PU_SetVal		(void)PWM_6_SetValue
#define AI_3_PU_ClrVal		(void)PWM_6_ClrValue
#define DO_20MA_1_SetVal	(void)PWM_7_SetValue
#define DO_20MA_1_ClrVal	(void)PWM_7_ClrValue
#define DO_20MA_2_SetVal	DO_1_SetVal
#define DO_20MA_2_ClrVal	DO_1_ClrVal
#define DO_HSD_SEN_SetVal	DO_2_SetVal
#define DO_HSD_SEN_ClrVal	DO_2_ClrVal

#define AI_CS_1		AD1_CHANNEL_AI_CS_1
#define AI_CS_2		AD1_CHANNEL_AI_CS_2
#define AI_CS_3		AD1_CHANNEL_AI_CS_3
#define AI_CS_4		AD1_CHANNEL_AI_CS_4
#define AI_OP_1		AD1_CHANNEL_AI_OP_1_CS_5
#define AI_OP_2		AD1_CHANNEL_AI_OP_2_CS_6
#define AI_OP_3		AD1_CHANNEL_AI_OP_3_CS7
#define AI_OP_4		AD1_CHANNEL_AI_OP_4
#define AI_1		AD1_CHANNEL_AI_1
#define AI_2		AD1_CHANNEL_AI_2
#define AI_3		AD1_CHANNEL_AI_3

#define AI_CHANNEL_COUNT	13

#endif // TARGET_7X

#ifdef TARGET_7H
#include "PWM_5.h"
#include "DO_1.h"
#include "DO_2.h"

#define DO_HSD_SEN2_SetVal	DO_1_SetVal
#define DO_HSD_SEN2_ClrVal	DO_1_ClrVal
#define DO_HSD_SEN1_SetVal	DO_2_SetVal
#define DO_HSD_SEN1_ClrVal	DO_2_ClrVal

#define AI_CS_1		AD1_CHANNEL_AI_CS_1
#define AI_CS_2		AD1_CHANNEL_AI_CS_2
#define AI_CS_3		AD1_CHANNEL_AI_CS_3
#define AI_CS_4		AD1_CHANNEL_AI_CS_4
#define AI_CS_5		AD1_CHANNEL_AI_OP_1_CS_5
#define AI_CS_6		AD1_CHANNEL_AI_OP_2_CS_6
#define AI_CS_7		AD1_CHANNEL_AI_OP_3_CS_7

#define AI_CHANNEL_COUNT	9

#endif // TARGET_7H

#ifdef TARGET_7L
#include "PWM_5.h"

#define AI_CHANNEL_COUNT	2

#endif // TARGET_7L

#endif /* IO_H_ */
