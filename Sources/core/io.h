/*
 * io.h
 *
 * Master pin I/O include; handles target-specific I/O
 * selection and remapping.
 */

#ifndef CORE_IO_H_
#define CORE_IO_H_

#include <core/mrs_bootrom.h>

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

// Superset I/Os
#include <DI_CAN_ERR.h>
#include <CAN_EN.h>
#include <CAN_STB_N.h>
#include <CAN_WAKE.h>
#include <DO_POWER.h>
#include <PWM_1.h>
#include <PWM_2.h>
#include <PWM_3.h>
#include <PWM_4.h>
#include <PWM_5.h>
#include <PWM_6.h>
#include <PWM_7.h>
#include <DO_1.h>
#include <DO_2.h>
#include <DO_30V_10V_1.h>
#include <DO_30V_10V_2.h>
#include <DO_30V_10V_3.h>

#define AI_3_PU_SetVal()		(void)PWM_6_SetValue()	// 7X
#define AI_3_PU_ClrVal()		(void)PWM_6_ClrValue()	// 7X
#define DO_20MA_1_SetVal()		(void)PWM_7_SetValue()	// 7X
#define DO_20MA_1_ClrVal()		(void)PWM_7_ClrValue()	// 7X
#define DO_20MA_2_SetVal()		DO_1_SetVal()			// 7X
#define DO_20MA_2_ClrVal()		DO_1_ClrVal()			// 7X
#define DO_HSD_SEN_SetVal()		DO_2_SetVal()			// 7X
#define DO_HSD_SEN_ClrVal()		DO_2_ClrVal()			// 7X

#define DO_HSD_SEN2_SetVal()	DO_1_SetVal()			// 7H
#define DO_HSD_SEN2_ClrVal()	DO_1_ClrVal()			// 7H
#define DO_HSD_SEN1_SetVal()	DO_2_SetVal()			// 7H
#define DO_HSD_SEN1_ClrVal()	DO_2_ClrVal()			// 7H

#define AI_KL15		AD1_CHANNEL_KL15				// 7X, 7H, 7L
#define AI_TEMP		AD1_CHANNEL_TEMP				// 7X, 7H, 7L
#define AI_CS_1		AD1_CHANNEL_AI_CS_1				// 7X, 7H
#define AI_CS_2		AD1_CHANNEL_AI_CS_2				// 7X, 7H
#define AI_CS_3		AD1_CHANNEL_AI_CS_3				// 7X, 7H
#define AI_CS_4		AD1_CHANNEL_AI_CS_4				// 7X, 7H
#define AI_CS_5		AD1_CHANNEL_AI_OP_1_CS_5		// 7H
#define AI_CS_6		AD1_CHANNEL_AI_OP_2_CS_6		// 7H
#define AI_CS_7		AD1_CHANNEL_AI_OP_3_CS_7		// 7H
#define AI_OP_1		AD1_CHANNEL_AI_OP_1_CS_5		// 7X
#define AI_OP_2		AD1_CHANNEL_AI_OP_2_CS_6		// 7X
#define AI_OP_3		AD1_CHANNEL_AI_OP_3_CS_7		// 7X
#define AI_OP_4		AD1_CHANNEL_AI_OP_4				// 7X
#define AI_1		AD1_CHANNEL_AI_1				// 7X
#define AI_2		AD1_CHANNEL_AI_2				// 7X
#define AI_3		AD1_CHANNEL_AI_3				// 7X

#define	AI_MAX		13
#define AI_NUM		((mrs_module_type == 'X') ? 13 : (mrs_module_type == 'H') ? 9 : 2)

#endif /* IO_H_ */
