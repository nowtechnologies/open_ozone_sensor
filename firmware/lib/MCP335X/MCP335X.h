// MCP335X driver, based on
// an Arduino program to read Microchip MCP3550-60 using SPI bus
// by John Beale www.bealecorner.com Feb. 9 2012

/*
===========================================================
MCP3550 is a differential input 22-bit ADC (21 bits + sign)
+Fullscale = (+Vref -1 LSB) = 0x1fffff
-Fullscale = (-Vref) = 0x200000
1 LSB => Vref / 2^21    for Vref=2 V, 1 LSB = 0.95 uV
Datasheet Spec: noise = 2.5 uV RMS with Vref = 2.5 V
===========================================================
*/

#ifndef MCP335X_H
#define MCP335X_H

#define MCP335X_MAX_VALUE 2097152

#include <Arduino.h>
#include <SPI.h>

union fourByteWord {
	unsigned long longv;
	byte c[4];
};	// allow access to 4-byte word, or each byte separately

class MCP335X {
private:
	int CS;   /// CS (out from Arduino), bring CS high for ADC sleep mode, low to start new conversion
	int MOSI; /// MOSI (data out from Arduino), MOSI is not used for this device
	int MISO; /// MISO (data in to Arduino), status and data bits from ADC
	int SCK;  /// SCK  (serial clock)
	unsigned long readWord(); /// read one word from 22-bit ADC device
	fourByteWord w;
	byte OVL;   /// overflow condition LOW
	byte OVH;   /// overflow condition HIGH
	unsigned int i;  /// loop counter for timeouts
	unsigned long x;
public:
	MCP335X(int CS, int MOSI, int MISO, int SCK);
	void begin();
	long read();
	long getLastValue();
	float getOzonePPM();
	float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
};

#endif // MCP335X_H
