/*
 * io.h
 *
 * Master pin I/O include; handles target-specific I/O
 * selection and remapping.
 */

#ifndef IO_H_
#define IO_H_

/**
 * Microplex 7 pin assignments across variants.
 * 
 * "Generic" names are assigned where a pin has different names between variants,
 * and then variant-specific macros below map the per-variant name onto the generic
 * name.
 * 
 * SoC Pin     Analog  Generic Name    7X              7H              7L
 * -------------------------------------------------------------------------------
 * A_0         0       AI_OP_1_CS_5    AI_OP_1         AI_CS_5         -
 * A_1         1       AI_OP_2_CS_6    AI_OP_2         AI_CS_6         -
 * A_2         2       AI_CS_2         AI_CS_2         AI_CS_2         -
 * A_3                 DO_1            DO_20MA_2       DO_HSD_SEN2     -
 * A_4                 DO_30V_10V_3    DO_30V_10V_3    -               -
 * A_5                 DO_2            DO_HSD_SEN      DO_HSD_SEN1     -
 * A_6         6       AI_2            AI_2            -               -
 * A_7         7       AI_3            AI_3            -               -
 * 
 * B_0         8       AI_OP_3_CS_7    AI_OP_3         AI_CS_7         -
 * B_1         9       AI_OP_4         AI_OP_4         -               -
 * B_2         10      AI_CS_1         AI_CS_1         AI_CS_1         -
 * B_3         11      AI_CS 3         AI_CS_3         AI_CS_3         -
 * B_4         12      AI_CS_4         AI_CS_4         AI_CS_4         -
 * B_5         13      AI_1            AI_1            -               -
 * B_6         14      AI_KL15         AI_KL15         AI_KL15         AI_KL15
 * 
 * D_0                 PWM_7           DO_20MA_1       DO_HSD_7        DO_LSD_7
 * D_2                 PWM_5           FREQ_IN         DO_HSD_5        DO_LSD_5
 * D_3                 PWM_6           AI_3_PU         DO_HSD_6        DO_LSD_6
 * D_4                 PWM_1           DO_HSD_1        DO_HSD_1        DO_LSD_1
 * D_5                 PWM_3           DO_HSD_3        DO_HSD_3        DO_LSD_3
 * D_6                 PWM_4           DO_HSD_4        DO_HSD_4        DO_LSD_4
 * D_7                 PWM_2           DO_HSD_2        DO_HSD_2        DO_LSD_2
 * 
 * E_0                 DO_30V_10V_2    DO_30V_10V_2    -               -
 * E_2                 DO_POWER        DO_POWER        DO_POWER        DO_POWER
 * E_5                 CAN_WAKE        CAN_WAKE        CAN_WAKE        CAN_WAKE
 * 
 * F_0                 CAN_EN          CAN_EN          CAN_EN          CAN_EN
 * F_2                 CAN_STB_N       CAN_STB_N       CAN_STB_N       CAN_STB_N
 * F_3                 DI_CAN_ERR      DI_CAN_ERR      DI_CAN_ERR      DI_CAN_ERR
 * F_5                 DO_30V_10V_1    DO_30V_10V_1    -               -
 *
 * XXX TODO something smart with FREQ_IN. It requires a different pin config,
 *          which isn't something Platform Expert is good at.
 */

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
