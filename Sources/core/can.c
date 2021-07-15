/*
 * CAN messaging.
 */

#include <CAN1.h>

#include <config.h>

#include <core/callbacks.h>
#include <core/can.h>
#include <core/lib.h>
#include <core/mrs_bootrom.h>
#include <core/pt.h>
#include <core/timer.h>

#include <can_devices/blink_keypad.h>

static can_buf_t            can_rx_fifo[CAN_RX_FIFO_SIZE];
static volatile uint8_t     can_buf_head;
static uint8_t              can_buf_tail;
#define _CAN_BUF_INDEX(_x)  ((_x) & (uint8_t)(CAN_RX_FIFO_SIZE - 1))
#define CAN_BUF_PTR(_x)     &can_rx_fifo[_CAN_BUF_INDEX(_x)]
#define CAN_BUF_EMPTY       (can_buf_head == can_buf_tail)
#define CAN_BUF_FULL        ((can_buf_head - can_buf_tail) >= CAN_RX_FIFO_SIZE)

void
_can_trace(uint8_t code)
{
    uint8_t b[1];
    uint8_t ret;

    b[0] = code;
    do {
        ret = CAN1_SendFrame(2, 0x0f, DATA_FRAME, 0x1, &b[0]);
    } while (ret != ERR_OK);
}

void
can_putchar(char ch)
{
    static uint8_t msg[8];
    static uint8_t len = 0;
    uint8_t ret;

    if (ch == '\n') {
        ch = '\0';
    }
    msg[len++] = (uint8_t)ch;
    if ((len == 8) || (ch == '\0')) {
        do {
            // send explicitly using buffer 0 to ensure messages are sent in order
            ret = CAN1_SendFrame(0, CAN_EXTENDED_FRAME_ID | 0x1ffffffe, DATA_FRAME, len, &msg[0]);
        } while (ret == ERR_TXFULL);
        len = 0;
    }
}

void
can_tx_async(uint32_t id, uint8_t dlc, const uint8_t *data)
{
    uint8_t ret;

    do {
        ret = CAN1_SendFrameExt(id, DATA_FRAME, dlc, (byte *)data);
    } while (ret == ERR_TXFULL);    
}

void
can_tx_ordered(uint32_t id, uint8_t dlc, const uint8_t *data)
{
    uint8_t ret;

    /* send explicitly using buffer 0 to ensure ordering */
    do {
        ret = CAN1_SendFrame(0, id, DATA_FRAME, dlc, (byte *)data);
    } while (ret == ERR_TXFULL);    
}

void
can_tx_blocking(uint32_t id, uint8_t dlc, const uint8_t *data)
{
    can_tx_ordered(id, dlc, data);

    /* and wait for buffer 0 to be done */
    while (CAN1_GetStateTX() ^ 0x01) {}
}

/*
 * Processor Expert doesn't give us a way to adjust the CAN bitrate,
 * and it seems to generate bogus clock config anyway, so fix it up here.
 */
void
can_reinit(uint8_t rate)
{
    /* Switch to initialization mode. */
    CANCTL0 |= CANCTL0_INITRQ_MASK;
    while (!(CANCTL1 & CANCTL1_INITAK_MASK)) {
    }

    /* Enable MSCAN, select external clock. */
    CANCTL1 = CANCTL1_CANE_MASK;

    /* Configure for the selected rate. */
    switch (rate) {
    case MRS_CAN_1000KBPS:
        CANBTR0 = 0x00;
        CANBTR1 = 0x05;
        break;
    case MRS_CAN_800KBPS:
        CANBTR0 = 0x00;
        CANBTR1 = 0x07;
        break;
    case MRS_CAN_500KBPS:
        CANBTR0 = 0x00;
        CANBTR1 = 0x1c;
        break;
    case MRS_CAN_250KBPS:
        CANBTR0 = 0x01;
        CANBTR1 = 0x1c;
        break;
    case MRS_CAN_125KBPS:
    default:
        CANBTR0 = 0x03;
        CANBTR1 = 0x1c;
        break;
    }

    /* 
     * XXX TODO Give the application a chance to tinker with the filters,
     *     but make sure that it can't disable the bootrom ID.
     */

    /* clear INITRQ and wait for it to be acknowledged */
    CANCTL0 ^= CANCTL0_INITRQ_MASK;
    while (CANCTL1 & CANCTL1_INITAK_MASK) {
    }
    CANRFLG |= 0xFE;                     /* Reset error flags */
    CANRIER = 0x01;                      /* Enable interrupts */

    /* clear the FIFO */
    can_buf_head = 0;
    can_buf_tail = 0;

    // now we can enable RX events
    CAN1_EnableEvent();
}

/*
 * Interrupt callback for CAN receive.
 */
void
can_rx_message(void)
{
    // check for a CAN message
    can_buf_t *buf;
    uint8_t type;
    uint8_t ret;
    uint8_t format;

    // If there's nowhere to put a message, just drop it.
    if (CAN_BUF_FULL) {
        can_trace(TRACE_CAN_IROVF);
        return;
    }
    buf = CAN_BUF_PTR(can_buf_head);

    // read the frame
    ret = CAN1_ReadFrame(&buf->id,
                         &type,
                         &format,
                         &buf->dlc,
                         &buf->data[0]);

    if ((ret == ERR_OK) &&
            (type == DATA_FRAME)) {

        if (((buf->id & MRS_ID_MASK) == MRS_ID_MASK) || app_can_filter(buf)) {
            // accept this message
            can_buf_head++;
            can_trace(TRACE_CAN_IRX);
        }
    }
}

can_buf_t *
can_buf_peek(void)
{
    return CAN_BUF_EMPTY ? NULL : CAN_BUF_PTR(can_buf_tail);
}

void
can_buf_drop(void)
{
    if (!CAN_BUF_EMPTY) {
        can_buf_tail++;
    }
}

void
can_listen(struct pt *pt)
{
    static timer_t  can_idle_timer;
    static bool     can_idle_flag = FALSE;
    uint8_t limit;


    pt_begin(pt);

    // set up the CAN idle timer
    timer_register(can_idle_timer);
    timer_reset(can_idle_timer, CAN_IDLE_TIMEOUT);

    for (;;) {
        // Limit the number of messages that will be processed to avoid watchdogging
        // during a message storm.
        limit = CAN_RX_FIFO_SIZE;

        while (!CAN_BUF_EMPTY && limit--) {
            can_buf_t *buf = CAN_BUF_PTR(can_buf_tail);
            can_trace(TRACE_CAN_TRX);

            // We're hearing CAN, so reset the idle timer and let the app know.
            timer_reset(can_idle_timer, CAN_IDLE_TIMEOUT);
            if (can_idle_flag) {
                can_idle_flag = FALSE;
                app_can_idle(FALSE);
            }

            // Handle MRS flasher messages directly.
            if ((buf->id & MRS_ID_MASK) == MRS_ID_MASK) {
                can_trace(TRACE_CAN_MRS_RX);
                mrs_bootrom_rx(buf);
            }

#ifdef CONFIG_WITH_BLINK_KEYPAD
            else if (bk_can_receive(buf)) {
                // keypad driver absorbed the packet
            }
#endif

            // Pass the message to the application.
            else {
                can_trace(TRACE_CAN_APP_RX);
                app_can_receive(buf);
            }

            // mark the slot as free
            can_buf_tail++;
        }

        // if we haven't heard a useful CAN message for a while...
        if (!can_idle_flag && timer_expired(can_idle_timer)) {
            can_idle_flag = TRUE;
            app_can_idle(TRUE);
        }

        pt_yield(pt);
    }
    pt_end(pt);
}
