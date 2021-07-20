/**
 * Interfaces to the MRS boot ROM and flash protocol.
 */

#ifndef CORE_MRS_BOOTROM_H_
#define CORE_MRS_BOOTROM_H_

#include <IEE1.h>

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

#define MRS_CAN_1000KBPS            1
#define MRS_CAN_800KBPS             2
#define MRS_CAN_500KBPS             3
#define MRS_CAN_250KBPS             4
#define MRS_CAN_125KBPS             5

/**
 * One of 'X', 'H' or 'L'; maps directly to the relevant character in
 * the 'Name' property.
 */
extern const char		mrs_module_type;

/**
 * Get the CAN bitrate from EEPROM.
 */
extern uint8_t   		mrs_can_bitrate(void);

/**
 * MRS CAN flash protocol handler.
 */
extern bool             mrs_bootrom_rx(can_buf_t *buf);

/**
 * MRS EEPROM contents.
 */
typedef struct {
	uint16_t	ParameterMagic;
	uint32_t	SerialNumber;
	char		PartNumber[12];
	char		DrawingNumber[12];
	char		Name[20];
	char		OrderNumber[8];
	char		TestDate[8];
	uint16_t	HardwareVersion;
	uint8_t		ResetCounter;
	uint16_t	LibraryVersion;
	uint8_t		ResetReasonLVD;
	uint8_t		ResetReasonLOC;
	uint8_t		ResetReasonILAD;
	uint8_t		ResetReasonILOP;
	uint8_t		ResetReasonCOP;
	uint8_t		MCUType;
	uint8_t		HardwareCANActive;
	uint8_t		_reserved1[3];
	uint16_t	BootloaderVersion;
	uint16_t	ProgramState;
	uint16_t	Portbyte1;
	uint16_t	Portbyte2;
	uint16_t	BaudrateBootloader1;
	uint16_t	BaudrateBootloader2;
	uint8_t		BootloaderIDExt1;
	uint32_t	BootloaderID1;
	uint8_t		BootloaderIDCRC1;
	uint8_t		BootloaderIDExt2;
	uint32_t	BootloaderID2;
	uint8_t		BootloaderIDCRC2;
	char		SoftwareVersion[20];
	char		ModuleName[30];
	uint8_t		BootloaderCANBus;
	uint16_t	COPWatchdogTimeout;
	uint8_t		_reserved2[7];
} mrs_parameters_t;

extern const mrs_parameters_t	mrs_parameters;

#endif /* MRS_BOOTROM_H_ */
