#ifndef __COMMON_PACKETS_H__
#define __COMMON_PACKETS_H__

#define PROTOCOL_REVISION  100

// UART HEADER
#define UART_LI_F 0x55
#define UART_LI_L 0xAA

// PACKET IDENTIFIERS
#define PID_ERROR  0
#define PID_SENSOR 1

#if PRAGMA_PACK_AVAILABLE
#define __PACKED
#pragma pack(push, 1)
#else
#define __PACKED __attribute__((packed))
#endif

// UART HEADER
typedef struct {
	uint8_t  firstLeadIn;
	uint8_t  lastLeadIn;
	uint8_t  packetLength;
	uint8_t  packetID;
	uint8_t  headerCRC;
} __PACKED UARTHEADER;

typedef struct {
	float ozonePPM; // 0
	float temperature; // 4
	float humidity; // 8
	float ratio; // 12
} __PACKED SENSORPACK;

#if PRAGMA_PACK_AVAILABLE
#pragma pack(pop)
#endif

#endif
