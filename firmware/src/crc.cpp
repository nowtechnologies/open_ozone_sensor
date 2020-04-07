#include "crc.h"
#include "Arduino.h"

uint8_t crc8(const void *buffer, size_t offset, size_t size) {
	int sum = 0;
	const uint8_t *p = (const uint8_t*)buffer + offset;
	while(size--) {
		sum += *p++;
	}
	return (~(sum & 0xff)) + 1;
}

/// Uses a simple 16-bit checksum calculaiton algorithm taken from
/// Cybro PLC protocols.
static uint16_t __primeTable[16] = {0x049D, 0x0C07, 0x1591, 0x1ACF, 0x1D4B, 0x202D, 0x2507, 0x2B4B, 0x34A5, 0x38C5, 0x3D3F, 0x4445, 0x4D0F, 0x538F, 0x5FB3, 0x6BBF};

uint16_t crc16(const void *buffer, size_t offset, size_t size) {
	uint16_t result = 0;
	const uint8_t *p = (const uint8_t*)buffer + offset;
	for(size_t i=0; i<size; i++) {
		result += (*p++ ^ 0x5A) * __primeTable[i & 0x0F];
	}
	return result;
}

void crc16Init(CRCContext &ctx) {
	ctx.prime_index = 0;
	ctx.checksum = 0;
}

void crc16Add(CRCContext &ctx, uint8_t data) {
	ctx.checksum += (data ^ 0x5A) * __primeTable[ctx.prime_index & 0x0F];
	ctx.prime_index++;
}
