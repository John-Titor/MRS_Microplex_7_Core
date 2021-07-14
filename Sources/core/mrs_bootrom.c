/*
 * Handle the firmware side (read-only) of the MRS flasher protocol.
 *
 * Messages handled at 0x1ffffff1:
 * 
 * receive              send
 * 00 00                00 id id id id st 00 vv     All-call "report your ID".
 * 20 00                2f ff id id id id 00 00     Enter program mode - sets EEPROM and resets.
 * 20 10 id id id id    21 10 id id id id 00 00     Select id id id id for subsequent operations.
 * 20 03 aa aa cc       dd ...                      EEPROM read cc (1-8) bytes from address aa aa.
 * 20 11 f3 33 af       21 11 01 00 00              EEPROM write enable
 * 20 02                20 f0 02 00 00              EEPROM write disable
 * 
 * EEPROM write data (specific locations only) is handled at 0x1ffffff5:
 * 
 * aa aa ...            20 e8 00 00 00              write eeprom data to aa aa
 *                      20 e8 0f 00 00				eeprom not unlocked
 *                      
 * There are lots of eeprom error messages, we just send the most generic one.
 */

#include <string.h>

#include <CAN1.h>
#include <IEE1.h>

#include <core/can.h>
#include <core/mrs_bootrom.h>
#include <core/lib.h>

#define	MRS_PARAM_BASE    IEE1_AREA_START

static bool     mrs_module_selected = FALSE;
static bool     mrs_eeprom_write_enable = FALSE;

static void     mrs_param_copy_bytes(uint16_t param_offset, uint8_t param_len, uint8_t *dst);
static bool     mrs_param_compare_bytes(uint16_t param_offset, uint8_t param_len, uint8_t *ref);
static void		mrs_param_store_bytes(uint16_t param_offset, uint8_t param_len, uint8_t *src);

typedef struct {
    uint8_t     id:4;
    uint8_t     len:4;
    uint8_t     cmd[5];
    void        (* func)(can_buf_t *buf);
} mrs_bootrom_handler_t;

static void     mrs_scan(can_buf_t *buf);
static void     mrs_select(can_buf_t *buf);

static const mrs_bootrom_handler_t  unselected_handlers[] = {
        { 0x1, 2, { 0x00, 0x00},                    mrs_scan },
        { 0x1, 2, { 0x20, 0x10},                    mrs_select }
};

static void     mrs_enter_program(can_buf_t *buf);
static void     mrs_read_eeprom(can_buf_t *buf);
static void     mrs_write_eeprom_enable(can_buf_t *buf);
static void     mrs_write_eeprom_disable(can_buf_t *buf);
static void     mrs_write_eeprom(can_buf_t *buf);

static const mrs_bootrom_handler_t  selected_handlers[] = {
        { 0x1, 2, { 0x20, 0x00},                    mrs_enter_program },
        { 0x1, 2, { 0x20, 0x03},                    mrs_read_eeprom },
        { 0x1, 5, { 0x20, 0x11, 0xf3, 0x33, 0xaf},  mrs_write_eeprom_enable },
        { 0x1, 2, { 0x20, 0x02},                    mrs_write_eeprom_disable },
        { 0x5, 0, { 0 },                            mrs_write_eeprom }
};

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

static bool
mrs_dispatch_handler(const mrs_bootrom_handler_t *handler, uint8_t table_len, can_buf_t *buf)
{
    while (table_len--) {
        if ((handler->id == (buf->id & 0xf)) &&             /* command matches */
                (handler->len <= buf->dlc) &&                   /* message is long enough */
                !memcmp(handler->cmd, buf->data, handler->len)) { /* prefix matches */

            handler->func(buf);
            return TRUE;
        }
        handler++;
    }
    return FALSE;
}

void
mrs_bootrom_rx(can_buf_t *buf)
{
    if (mrs_dispatch_handler(unselected_handlers, 
                             sizeof(unselected_handlers) / sizeof(mrs_bootrom_handler_t),
                             buf)) {
        return;
    }
    if (mrs_module_selected
            && mrs_dispatch_handler(selected_handlers, 
                                    sizeof(selected_handlers) / sizeof(mrs_bootrom_handler_t),
                                    buf)) {
        return;
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
    mrs_param_copy_bytes(MRS_PARAM_ADDR_BL_VERS + 1, 1, &data[7]); /* low byte only */
    can_tx_ordered(MRS_SCAN_RSP_ID | CAN_EXTENDED_FRAME_ID,
                   sizeof(data),
                   &data[0]);

    mrs_module_selected = FALSE;
    mrs_eeprom_write_enable = FALSE;
}

void 
mrs_enter_program(can_buf_t *buf)
{
    uint8_t data[8] = {0x2f, 0xff};
    (void)buf;

    can_trace(TRACE_MRS_PROGRAM);

    /* send the 'will reset' message */
    mrs_param_copy_bytes(MRS_PARAM_ADDR_SERIAL, 4, &data[2]);
    can_tx_blocking(MRS_RESPONSE_ID | CAN_EXTENDED_FRAME_ID,
                    sizeof(data),
                    &data[0]);

    /* XXX set EEPROM to "boot to bootloader" mode? */

    /* XXX reset immediately */
    __asm DCW 0x9e00;
}

void
mrs_select(can_buf_t *buf)
{
    uint8_t data[8] = {0x21, 0x10};

    /* verify that this message is selecting this module */
    if (!mrs_param_compare_bytes(MRS_PARAM_ADDR_SERIAL, 4, &buf->data[2])) {

        /* someone else got selected, we should be quiet now */
        mrs_module_selected = FALSE;
        return;
    }

    can_trace(TRACE_MRS_SELECT);

    /* send the 'selected' response */
    mrs_param_copy_bytes(MRS_PARAM_ADDR_SERIAL, 4, &data[2]);
    /* note no bootloader version */
    can_tx_ordered(MRS_RESPONSE_ID | CAN_EXTENDED_FRAME_ID,
                   sizeof(data),
                   &data[0]);

    mrs_module_selected = TRUE;
}

void
mrs_read_eeprom(can_buf_t *buf)
{
    const uint16_t param_offset = ((uint16_t)buf->data[2] << 8) | buf->data[3];
    const uint8_t param_len = buf->data[4];
    uint8_t data[8];

    can_trace(TRACE_MRS_GET_PARAM);

    mrs_param_copy_bytes(param_offset, param_len, &data[0]);
    can_tx_ordered(MRS_EEPROM_READ_ID | CAN_EXTENDED_FRAME_ID,
                   param_len,
                   &data[0]);
}

void
mrs_write_eeprom_enable(can_buf_t *buf)
{
    static const uint8_t data[5] = {0x21, 0x11, 0x01, 0x00, 0x00};

    (void)buf;
    can_trace(TRACE_MRS_EEPROM_ENABLE);

    mrs_eeprom_write_enable = TRUE;
    can_tx_ordered(MRS_RESPONSE_ID | CAN_EXTENDED_FRAME_ID,
                   sizeof(data),
                   &data[0]);
}

void
mrs_write_eeprom_disable(can_buf_t *buf)
{
    static const uint8_t data[5] = {0x20, 0xF0, 0x02, 0x00, 0x00};

    (void)buf;
    can_trace(TRACE_MRS_EEPROM_DISABLE);

    mrs_eeprom_write_enable = FALSE;
    can_tx_ordered(MRS_RESPONSE_ID | CAN_EXTENDED_FRAME_ID,
                   sizeof(data),
                   &data[0]);
}

void
mrs_write_eeprom(can_buf_t *buf)
{
    const uint16_t address = ((uint16_t)buf->data[0] << 8) | buf->data[1];
    const uint8_t len = buf->dlc - 2;
    uint8_t * const src = &buf->data[2];
    uint8_t data[5] = {0x20, 0xe8, 0x0f};	// default to error

    can_trace(TRACE_MRS_EEPROM_WRITE);

    if (mrs_eeprom_write_enable) {

        // CAN bitrate update?
        if ((address == MRS_PARAM_CAN_RATE_1) &&	// speed 2 is auto-updated only 
                (len == 2) && 							// write just this value
                ((src[0] ^ src[1]) == 0xff) &&			// check code is OK
                (src[1] >= MRS_CAN_1000KBPS) &&			// value is within bounds
                (src[1] <= MRS_CAN_125KBPS)) {

            // write backup copy, and approve this write
            mrs_param_store_bytes(MRS_PARAM_CAN_RATE_2, 2, src);
            data[2] = 0;
        }

        // write to "user" area of the EEPROM (first page only)
        else if ((address >= 0x200) &&
                (address < 0x400) &&
                ((address + len) <= 0x400)) {
            data[2] = 0;
        }
    }

    // if write was approved
    if (data[2] == 0) {
        mrs_param_store_bytes(address, len, src);
    }

    // and send response
    can_tx_ordered(MRS_RESPONSE_ID | CAN_EXTENDED_FRAME_ID,
                   sizeof(data),
                   &data[0]);
}

static void
mrs_param_copy_bytes(uint16_t param_offset, uint8_t param_len, uint8_t *dst)
{
    while (param_len--) {
        (void)IEE1_GetByte(MRS_PARAM_BASE + param_offset++, dst++);
    }
}

static bool
mrs_param_compare_bytes(uint16_t param_offset, uint8_t param_len, uint8_t *ref)
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

static void
mrs_param_store_bytes(uint16_t param_offset, uint8_t param_len, uint8_t *src)
{
    while (param_len--) {
        (void)IEE1_SetByte(MRS_PARAM_BASE + param_offset++, *src++);
    }
}
