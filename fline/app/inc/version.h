#ifndef VERSION_H__
#define VERSION_H__

/* clang-format off */
#define     FIRMWARE_BRANCH_NAME_SIZE   12
#define     FIRMWARE_BRANCH_NAME        "f/sigfox-wsg"
#define     FIRMWARE_BRANCH_SHA         0x78d4083c

#define     FIRMWARE_TYPE_DEV           0xFF
#define     FIRMWARE_TYPE_PROD          0x00

#define     FIRMWARE_VERSION_MAJOR      0
#define     FIRMWARE_VERSION_MINOR      0
#define     FIRMWARE_VERSION_PATCH      4

#ifdef DEBUG
	#define     FIRMWARE_TYPE       FIRMWARE_TYPE_DEV
#else
	#define     FIRMWARE_TYPE       FIRMWARE_TYPE_PROD
#endif

#define     HARDWARE_REV                1
/* clang-format on */
#endif
