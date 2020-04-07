#define UART_LI_F 0x55
#define UART_LI_L 0xAA
#define UART_BUFFER_SIZE 128

#include "ringbuffer.h"
#include "packets.h"

RingBuffer serialBuffer(UART_BUFFER_SIZE);
UARTHEADER packetHeader;
SENSORPACK sensorPacket;

void process(const UARTHEADER &header, RingBuffer &buffer) {

  if(header.packetID == PID_SENSOR) {
    if(buffer.capacity() >= sizeof(SENSORPACK)) {
      buffer.get(0, (uint8_t*)&sensorPacket, sizeof(SENSORPACK));
    }
  }
}

void append(uint8_t b) {
	uint16_t datacrc;
	uint16_t checksum;

	if(serialBuffer.push(b)) {
		size_t required = sizeof(UARTHEADER) + packetHeader.packetLength;

		while (serialBuffer.capacity() >= required) {
			// header bytes available
			serialBuffer.get(0, (uint8_t*)&packetHeader, sizeof(UARTHEADER));

			if(packetHeader.firstLeadIn == UART_LI_F && packetHeader.lastLeadIn == UART_LI_L) {
				uint8_t crc = crc8(&packetHeader, 0, sizeof(UARTHEADER) - 1);
				if(packetHeader.headerCRC == crc) {
					if(serialBuffer.capacity() >= sizeof(UARTHEADER) + packetHeader.packetLength) {
							// data bytes available
							serialBuffer.pop(sizeof(UARTHEADER));

							serialBuffer.get(packetHeader.packetLength - 2, (uint8_t*)&datacrc, sizeof(uint16_t));
							checksum = serialBuffer.crc16(0, packetHeader.packetLength - 2);
							if(datacrc == checksum) {
								process(packetHeader, serialBuffer); // [1716 byte]
							} else {
								// data crc error - read next packet
							}

							serialBuffer.pop(packetHeader.packetLength);
							packetHeader.packetLength = 0;
					} else {
						// not enough bytes available (continue read)
						break;
					} // if enough bytes
				} else {
					// error - invalid crc
					packetHeader.packetLength = 0;
					serialBuffer.pop();
				} // if crc
			} else {
				// invalid lead in (continue read)
				packetHeader.packetLength = 0;
				serialBuffer.pop();
			} // if lead in
		} // while
	} else {
		// error - buffer overflow
		serialBuffer.clear();
	} // push
}

void send(uint8_t pid, const void *data, size_t length) {
	static UARTHEADER header;
	header.firstLeadIn  = UART_LI_F;
	header.lastLeadIn   = UART_LI_L;
	header.packetLength	= (uint8_t)length + 2; // +2 bytes for crc16
	header.packetID			= pid;
	header.headerCRC		= crc8(&header, 0, sizeof(UARTHEADER) - 1);
  // write Header
	for (size_t i = 0; i < sizeof(UARTHEADER); ++i) {
		Serial.write(((uint8_t*)&header)[i]);
	}
	// write Data
	for (size_t i = 0; i < length; ++i) {
		Serial.write(((uint8_t*)data)[i]);
	}
	// Write Data CRC
	uint16_t crc = crc16(data, 0, length);
	Serial.write((uint8_t)(crc & 0x00ff)); // LSB
	Serial.write((uint8_t)((crc & 0xff00) >> 8)); // MSB
}
