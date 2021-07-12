/*
 * Interfaces to the MRS boot ROM and flash protocol.
 */

#ifndef MRS_BOOTROM_H_
#define MRS_BOOTROM_H_

#include "can.h"

#define MRS_FLASHER_ID      0x1ffffff1
#define MRS_RESPONSE_ID     0x1ffffff2
#define MRS_DATA_ID         0x1ffffff4

#define MRS_PARAM_ADDR_SERIAL       2
#define MRS_PARAM_ADDR_BL_VERS      83
#define MRS_PARAM_ADDR_PGM_STATE    85
#define MRS_PARAM_CAN_RATE_1        91
#define MRS_PARAM_CAN_RATE_2        93

typedef enum {
    MRS_CAN_1000KBPS    = 1,
    MRS_CAN_800KBPS,
    MRS_CAN_500KBPS,
    MRS_CAN_250KBPS,
    MRS_CAN_125KBPS
} mrs_can_rate_t;

/**
 * Get the CAN bitrate from EEPROM.
 */
extern mrs_can_rate_t   mrs_can_bitrate(void);

/**
 * MRS CAN flash protocol handler.
 */
extern void             mrs_flash_rx(can_buf_t *buf);

#endif /* MRS_BOOTROM_H_ */
