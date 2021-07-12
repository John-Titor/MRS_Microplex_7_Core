/*
 * CAN interfaces.
 */

#ifndef CAN_H_
#define CAN_H_

#include "lib.h"

/**
 * CAN message structure.
 */
typedef struct {
    uint32_t    id;
    uint8_t     dlc;
    uint8_t     data[8];
} can_buf_t;

/**
 * Sends a single-byte CAN trace message.
 *
 * @param code		The trace message payload.
 */
extern void can_trace(uint8_t code);

/**
 * Adds a single character to the CAN console buffer.
 * 
 * If the character is '\n', or the buffer is full, a
 * console message will be sent.
 * 
 * @param ch		The character to add.
 */
extern void can_putchar(char ch);

/**
 * Platform Expert generates bogus CAN configuration. Fix it.
 */
extern void can_reinit(void);

/**
 * Interrupt callback; copies CAN messages into the RX FIFO.
 */
extern void can_rx_message(void);

/**
 * CAN listener thread. Call this periodically to process the
 * RX FIFO.
 * 
 * @param pt		Callback protothread.
 */
extern void can_listen(struct pt *pt);

#endif /* CAN_H_ */
