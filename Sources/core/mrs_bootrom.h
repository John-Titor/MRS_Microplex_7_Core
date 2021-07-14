/**
 * Interfaces to the MRS boot ROM and flash protocol.
 */

#ifndef CORE_MRS_BOOTROM_H_
#define CORE_MRS_BOOTROM_H_

#include <core/can.h>

#define MRS_ID_MASK                 0x1ffffff0      // XXX TODO fetch from EEPROM
#define MRS_SCAN_RSP_ID             0x1ffffff0
#define MRS_COMMAND_ID              0x1ffffff1
#define MRS_RESPONSE_ID             0x1ffffff2
#define MRS_EEPROM_READ_ID          0x1ffffff4
#define MRS_EEPROM_WRITE_ID         0x1ffffff5

#define MRS_PARAM_ADDR_SERIAL       0x04
#define MRS_PARAM_ADDR_BL_VERS      0x53
#define MRS_PARAM_CAN_RATE_1        0x5b
#define MRS_PARAM_CAN_RATE_2        0x5d

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
extern void             mrs_bootrom_rx(can_buf_t *buf);

#endif /* MRS_BOOTROM_H_ */
