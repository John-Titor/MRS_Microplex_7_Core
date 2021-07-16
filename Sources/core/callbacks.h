/*
 * Application callbacks.
 */

#ifndef CORE_CALLBACKS_H_
#define CORE_CALLBACKS_H_

#include <core/can.h>
#include <core/lib.h>

/**
 * Application CAN filter.
 * 
 * Called at interrupt time when a message is received.
 * 
 * @param buf		CAN message.
 * @return			TRUE if the message should be queued for processing,
 * 					FALSE if it has been handled or should be discarded.
 */
extern bool app_can_filter(can_buf_t *buf);

/**
 * Application CAN receive callback.
 * 
 * Called when a message is received but not handled by an internal
 * function.
 * 
 * @param buf       CAN message.
 */
extern void app_can_receive(can_buf_t *buf);

/**
 * Application CAN timeout.
 * 
 * Called on the CAN protothread when CAN traffic either stops or starts.
 * 
 * @bool is_idle        TRUE if the CAN bus has gone idle, FALSE 
 *                      if traffic has resumed.
 */
extern void app_can_idle(bool is_idle);

/**
 * ADC cycle complete callback.
 * 
 * Called at interrupt time when a complete ADC conversion cycle has
 * completed and new data is ready to fetch.
 */

extern void app_adc_ready(void);

#endif /* CALLBACKS_H_ */
