#include "MCP335X.h"

MCP335X::MCP335X(int CS, int MOSI, int MISO, int SCK) : CS(CS), MOSI(MOSI), MISO(MISO), SCK(SCK) {
	w.longv = 0u;
	OVL = 0;
	OVH = 0;
	i = 0;
	x = 0;
}

void MCP335X::begin() {
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV64); // SPI clock rate < 5 MHz per MCP3550 spec
	SPI.setBitOrder(MSBFIRST);            // MSB or LSB first
	SPI.setDataMode(SPI_MODE3);           // rising/falling edge of clock
	digitalWrite(CS, HIGH);
	pinMode(CS,   OUTPUT);
	pinMode(MOSI, OUTPUT);
	pinMode(MISO, INPUT);
	pinMode(SCK,  OUTPUT);
}

unsigned long MCP335X::readWord() {
	w.c[3] = SPI.transfer(0x00);	// fill 3 bytes with data: 22 bit signed int + 2 overflow bits
	w.c[2] = SPI.transfer(0x00);
	w.c[1] = SPI.transfer(0x00);
	w.c[0] = 0x00;              	// low-order byte set to zero
	return (w.longv);               // return unsigned long word
}

long MCP335X::read() {
	digitalWrite(CS,HIGH);
	delayMicroseconds(100);
	digitalWrite(CS,LOW); // start next conversion
	delay(50);            // delay in milliseconds (nominal MCP3550-60 rate: 66.7 msec => 15 Hz)
	i=0;                  // use i as loop counter
	do {
		i++;
		delayMicroseconds(50);                            // loop keeps trying for up to 1 second
	} while ((digitalRead(MISO)==HIGH) && (i < 20000));   // wait for ready bit to drop low (ready)
	unsigned long w = readWord();    	 // data in:  32-bit word gets 24 bits via SPI port
	OVL = ((w & 0x800000) != 0x000000);  // ADC negative overflow bit (input > +Vref)
	OVH = ((w & 0x400000) != 0x000000);  // ADC positive overflow bit (input < -Vref)
	x = w << 2;  // to use the sign bit
	x = x/1024;  // to move the LSB to bit 0 position
	return x;
}

long MCP335X::getLastValue() {
	return x;
}

float MCP335X::mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

float MCP335X::getOzonePPM(){
	float o3 = mapfloat(x, 0, MCP335X_MAX_VALUE, 10, 1000);
	if (o3 < 10) return 0; else return o3;
}
