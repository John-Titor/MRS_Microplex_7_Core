/*
 * Application callbacks.
 */

#ifndef CALLBACKS_H_
#define CALLBACKS_H_

#include "can.h"
#include "lib.h"

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
