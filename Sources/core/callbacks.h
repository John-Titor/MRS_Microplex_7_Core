/*
 * Application callbacks.
 */

#ifndef CORE_CALLBACKS_H_
#define CORE_CALLBACKS_H_

#include <core/can.h>
#include <core/lib.h>

/**
 * Application CAN filter; called at interrupt time when a message
 * is received.
 * 
 * @param buf		CAN message.
 * @return			TRUE if the message should be queued for processing,
 * 					FALSE if it has been handled or should be discarded.
 */
extern bool app_can_filter(can_buf_t *buf);

/**
 * Application CAN receive; called when a message is received
 * but not handled by the framework.
 */
extern void app_can_receive(can_buf_t *buf);

/**
 * Application CAN timeout: called when CAN traffic either stops
 * or starts.
 */
extern void app_can_idle(bool is_idle);

#endif /* CALLBACKS_H_ */
