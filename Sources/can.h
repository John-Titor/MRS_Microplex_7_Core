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

/** set this bit in the id field to send a 29-bit id */
#define CAN_ID_EXT      0x80000000UL

/**
 * Sends a single-byte CAN trace message.
 *
 * @param code		The trace message payload.
 */
#if 1
#define can_trace(_x)   _can_trace(_x)
#else
#define can_trace(_x)   do {} while(0);
#endif
extern void _can_trace(uint8_t code);

#define TRACE_POWER_ON      0xff
#define TRACE_CAN_IRX       0xf0    // interrupt received-and-queued
#define TRACE_CAN_IROVF     0xf1    // interrupt received-and-overflowed
#define TRACE_CAN_TRX       0xf2    // thread received message
#define TRACE_CAN_MRS_RX    0xf3    // message for mrs_bootrom code
#define TRACE_CAN_APP_RX    0xf4    // message for app

#define TRACE_MRS_BADMSG    0xe0    // unhandled / rejected message
#define TRACE_MRS_SCAN      0xe1
#define TRACE_MRS_PROGRAM   0xe2
#define TRACE_MRS_SELECT    0xe3
#define TRACE_MRS_GET_PARAM 0xe4

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
 * Send a CAN message.
 *
 * Returns as soon as the message has been queued in
 * any transmit slot. Messages queued with this interface
 * may not be transmitted in order.
 * 
 * @param buf       The message to send.
 */
extern void can_tx_async(can_buf_t *buf);

/**
 * Send a CAN message, explicitly ordered against other 
 * messages sent with this interface.
 * 
 * Returns once the message is queued for transmission;
 * will wait until any other message queued by this 
 * API has been sent.
 * 
 * @param buf       The message to send.
 */
extern void can_tx_ordered(can_buf_t *buf);

/**
 * Send a CAN message and wait for it to be sent.
 */
extern void can_tx_blocking(can_buf_t *buf);

/**
 * (Re)configure the CAN hardware.
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
