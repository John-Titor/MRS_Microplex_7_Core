/*
 * Handle the firmware side (read-only) of the MRS flasher protocol.
 *
 * Messages handled:
 * 
 * receive              send
 * 00 00                00 id id id id st 00 vv     All-call "report your ID".
 * 20 00                2f ff id id id id 00 00     Enter program mode - sets EEPROM and resets.
 * 20 10 id id id id    21 10 id id id id 00 00     Select id id id id for subsequent operations.
 * 20 03 aa aa cc       dd ...                      EEPROM read cc (1-8) bytes from address aa aa.
 */

#include <CAN1.h>
#include <IEE1.h>

#include "can.h"
#include "mrs_bootrom.h"
#include "lib.h"

#define	MRS_PARAM_BASE    IEE1_AREA_START

static bool     mrs_module_selected = FALSE;

static void     mrs_param_copy_bytes(uint8_t param_offset, uint8_t param_len, uint8_t *dst);
static bool     mrs_param_compare_bytes(uint8_t param_offset, uint8_t param_len, uint8_t *ref);

static void     mrs_scan(can_buf_t *buf);
static void     mrs_enter_program(can_buf_t *buf);
static void     mrs_select(can_buf_t *buf);
static void     mrs_read_eeprom(can_buf_t *buf);

static mrs_can_rate_t
_mrs_can_try_bitrate(uint8_t addr)
{
    uint8_t rate_code;
    uint8_t check_code;

    (void)IEE1_GetByte(MRS_PARAM_BASE + addr, &check_code);
    (void)IEE1_GetByte(MRS_PARAM_BASE + addr + 1, &rate_code);

    if ((rate_code ^ check_code) == 0xff) {
        return rate_code;
    }
    return 0;
}

mrs_can_rate_t
mrs_can_bitrate(void)
{
    mrs_can_rate_t  rate;

    /* try each of the configured CAN bitrate sets */
    rate = _mrs_can_try_bitrate(MRS_PARAM_CAN_RATE_1);
    if (rate != 0) {
        return rate;
    }
    rate = _mrs_can_try_bitrate(MRS_PARAM_CAN_RATE_2);
    if (rate != 0) {
        return rate;
    }
    return MRS_CAN_125KBPS;
}

static const struct {
    uint8_t     cmd[2];
    void        (* func)(can_buf_t *buf);
} handler[] = {
    { { 0x00, 0x00},    mrs_scan },
    { { 0x20, 0x00},    mrs_enter_program },
    { { 0x20, 0x10},    mrs_select },
    { { 0x20, 0x03},    mrs_read_eeprom },
    { {0}, NULL }
};

void
mrs_bootrom_rx(can_buf_t *buf)
{
    /* messages always at least 2 long */
    if (buf->dlc >= 2) {
        uint8_t i;

        /* look for a handler */
        for (i = 0; handler[i].func != 0; i++) {
            can_trace(i);
            if ((handler[i].cmd[0] == buf->data[0]) && (handler[i].cmd[1] == buf->data[1])) {
                handler[i].func(buf);
                return;
            }
        }
    }
    can_trace(TRACE_MRS_BADMSG);
}

void
mrs_scan(can_buf_t *buf)
{
    uint8_t data[8] = {0};
    (void)buf;

    can_trace(TRACE_MRS_SCAN);

    /* send the scan response message */
    mrs_param_copy_bytes(MRS_PARAM_ADDR_SERIAL, 4, &data[1]);
    mrs_param_copy_bytes(MRS_PARAM_ADDR_BL_VERS, 1, &data[7]);
    (void)CAN1_SendFrameExt(MRS_SCAN_RSP_ID | CAN_EXTENDED_FRAME_ID,
                            DATA_FRAME,
                            sizeof(data),
                            &data[0]);

    mrs_module_selected = FALSE;
}

void 
mrs_enter_program(can_buf_t *buf)
{
    uint8_t data[8] = {0x2f, 0xff};
    (void)buf;

    /* ignore unless we have been selected */
    if (!mrs_module_selected) {
        return;
    }

    can_trace(TRACE_MRS_PROGRAM);

    /* send the 'will reset' message */
    mrs_param_copy_bytes(MRS_PARAM_ADDR_SERIAL, 4, &data[2]);
    (void)CAN1_SendFrameExt(MRS_RESPONSE_ID | CAN_EXTENDED_FRAME_ID,
                            DATA_FRAME,
                            sizeof(data),
                            &data[0]);

    /* XXX wait / check for tx complete */

    /* XXX set EEPROM to "boot to program" mode */

    /* XXX reset immediately (illegal opcode?) */
}

void
mrs_select(can_buf_t *buf)
{
    uint8_t data[8] = {0x21, 0x10};

    /* verify that this message is selecting this module */
    if (!mrs_param_compare_bytes(MRS_PARAM_ADDR_SERIAL, 4, &buf->data[2])) {
        return;
    }

    can_trace(TRACE_MRS_SELECT);

    /* send the 'selected' response */
    mrs_param_copy_bytes(MRS_PARAM_ADDR_SERIAL, 4, &data[2]);
    (void)CAN1_SendFrameExt(MRS_RESPONSE_ID | CAN_EXTENDED_FRAME_ID,
                            DATA_FRAME,
                            sizeof(data),
                            &data[0]);

    mrs_module_selected = TRUE;
}

void
mrs_read_eeprom(can_buf_t *buf)
{
    const uint8_t param_offset = buf->data[3];
    const uint8_t param_len = buf->data[4];
    uint8_t data[8];

    /* ignore unless we have been selected */
    if (!mrs_module_selected) {
        return;
    }

    can_trace(TRACE_MRS_GET_PARAM);

    /* ignore high parameter address byte, it's always zero */
    mrs_param_copy_bytes(param_offset, param_len, &data[0]);
    (void)CAN1_SendFrameExt(MRS_DATA_ID | CAN_EXTENDED_FRAME_ID,
                            DATA_FRAME,
                            param_len,
                            &data[0]);
}

static void
mrs_param_copy_bytes(uint8_t param_offset, uint8_t param_len, uint8_t *dst)
{
    while (param_len--) {
        (void)IEE1_GetByte(MRS_PARAM_BASE + param_offset++, dst++);
    }
}

static bool
mrs_param_compare_bytes(uint8_t param_offset, uint8_t param_len, uint8_t *ref)
{
    while (param_len--) {
        uint8_t v;

        (void)IEE1_GetByte(MRS_PARAM_BASE + param_offset++, &v);
        if (v != *ref++) {
            return FALSE;
        }
    }
    return TRUE;
}
