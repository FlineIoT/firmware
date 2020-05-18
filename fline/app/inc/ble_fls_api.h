#pragma once

#define BLE_API_VERSION 1

/****
 * Control Characterstic API documentation 
 * Opcode usage: 
 *  R-Read: Property can be read
 *      To perform a property read: {OPCODE, 0}
 *      A response will be sent back in the form of {OPCODE | FLS_CONTROL_RESPONSE_BIT, length, data}
 *  W-Write: Property can be written
 *      To perform a property write: {OPCODE, length, data}
 *  C-Command: Property is a command, some commands expect a status response, see doc
 *      To send a command: {OPCODE, 0}
 *  I-Indicate: Property will be sent to peer as it changes (Notify)
 * Error response for any wrong control packet: {OPCODE | FLS_CONTROL_RESPONSE_BIT, 0}
*****/

#define FLS_CONTROL_RESPONSE_BIT        0x80
#define FLS_CONTROL_DATA_MAX_LEN        18

//Control structures
typedef uint8_t fls_opcode_t;
typedef struct __attribute__((packed))
{
    fls_opcode_t opcode;
    uint8_t len;
    uint8_t data[FLS_CONTROL_DATA_MAX_LEN];
} fls_control_t;

typedef struct __attribute__((packed))
{
    uint8_t fw_major;
    uint8_t fw_minor;
    uint8_t fw_patch;
    uint8_t fw_type;
    uint8_t hw_rev;
    uint8_t bl_rev;
} fline_rev_t;

//////// Device System Properties
#define PERIPHERAL_SYS_TIME				0x01 	//			// (W) data     | uint32_t time : Current real time (Set this to sync time)
#define PERIPHERAL_SYS_UPTIME			0x02 	//			// (R) response | uint32_t uptime : Time in seconds since boot
#define PERIPHERAL_SYS_REVISION         0x03 	//			// (R) response | fline_rev_t rev : Revision
#define PERIPHERAL_SYS_SERIAL			0x04 	//			// (R) response | uint32_t deviceID : Serial Number

/////// Device System Commands
#define PERIPHERAL_SYS_RESET			0x10 	//		 	// (C) null | Reset Device
#define PERIPHERAL_SYS_BOOTLOADER		0x11 	//			// (C) null | Reset and enter bootloader (TODO if error response (opcode 0x91), it means battery is too low)
