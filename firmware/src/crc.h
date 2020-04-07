#ifndef _CRC_H
#define _CRC_H

#include <stdlib.h>
#include <stdint.h>

typedef struct {
	uint8_t prime_index;
	uint16_t checksum;
} CRCContext;

uint8_t crc8(const void *buffer, size_t offset, size_t size);
uint16_t crc16(const void *buffer, size_t offset, size_t size);
void crc16Init(CRCContext &ctx);
void crc16Add(CRCContext &ctx, uint8_t data);

#endif
