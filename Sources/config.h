/*
 * Configuration parameters.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* 
 * Timeout (ms) before deciding that CAN has been disconnected.
 */
#define CAN_IDLE_TIMEOUT            2000

/*
 * Size of the CAN receive FIFO.
 */
#define CAN_RX_FIFO_SIZE			8

/*
 * Minimum load current (mA): below this, output is considered open.
 */
#define SENSE_OPEN_CURRENT          50

/*
 * Maximum load current (mA): over this, output is considered overloaded.
 */
#define SENSE_OVERLOAD_CURRENT      2500

/*
 * Maximum off voltage (mV): over this, output is considered stuck/shorted to +12.
 */
#define SENSE_STUCK_VOLTAGE         2000

/*
 * Inrush current settling time (ms)
 */
#define SENSE_INRUSH_DELAY          50

/*
 * Turn-off current settling time (ms)
 */
#define SENSE_SETTLE_DELAY          500

/*
 * Delay between retries for an overloaded output (ms).
 */
#define SENSE_OVERLOAD_RETRY_INTERVAL   1000

// ADC scale factors
//
// Measurements in 10-bit mode.
//
// Scaling is performed by taking the accumulated ADC counts
// (sum of ADC_AVG_SAMPLES), multiplying by the scale factor
// and then right-shifting by 12, i.e. the scale factor is a
// 4.12 fixed-point quantity.
//
// To calculate the scaling factor, take mV-per-count and
// multiply by 512.
//
// Current sense outputs are the same but for mA.
//
// AI_1/2/3:
// --------
// 1K pullup mode: TBD
// 20mA mode: TBD (claimed 25mA)

#define ADC_SCALE_FACTOR_30V    17900U  // VALIDATED @ 4.860V
#define ADC_SCALE_FACTOR_10V    6065U   // VALIDATED @ 4.860V

// AI_OP_1/2/3/4:
// -------------

#define ADC_SCALE_FACTOR_DO_V   16494U  // VALIDATED @ 11.46V

// AI_CS_1/2/3/4:
// -------------

#define ADC_SCALE_FACTOR_DO_I   4531U   // VALIDATED @ 1.000A

// AI_KL15:
// -------
// Clamped at 11V; mostly useful to help detect input sag and
// avoid faulting outputs when T30 is low.
//

#define ADC_SCALE_FACTOR_KL15   5507U   // VALIDATED @ 8.368V

// AI_TEMP
// -------
// Calculated for nominal Vdd (5V)

#define ADC_SCALE_FACTOR_TEMP   610U    // XXX VALIDATE

#endif // _CONFIG_H
