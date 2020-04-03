// Arduino program to read Microchip MCP3550-60 using SPI bus
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
#include <Arduino.h>
#include <SPI.h> // use Arduino SPI library

// #define CS    3
// #define MOSI 11
// #define MISO 12
// #define SCK  13

#define CS   10     // bring CS high for ADC sleep mode, low to start new conversion
#define MOSI 11   // MOSI is not used for this device
#define MISO 12  // status and data bits from ADC
#define SCK  13   // SPI clock from Arduino to ADC

/**
 * read one word from 22-bit ADC device  MCP3550
 */
unsigned long readword() {
    union {
        unsigned long svar;
        byte c[4];
    } w;    // allow access to 4-byte word, or each byte separately
    w.c[3] = SPI.transfer(0x00);    // fill 3 bytes with data: 22 bit signed int + 2 overflow bits
    w.c[2] = SPI.transfer(0x00);
    w.c[1] = SPI.transfer(0x00);
    w.c[0] = 0x00;                  // low-order byte set to zero
    return(w.svar);    // return unsigned long word
}

void setup() {
 Serial.begin(115200);
 SPI.begin();
 SPI.setClockDivider(SPI_CLOCK_DIV4);  // SPI clock rate < 5 MHz per MCP3550 spec
 SPI.setBitOrder(MSBFIRST);     // MSB or LSB first
 SPI.setDataMode(SPI_MODE3);        // rising/falling edge of clock
 pinMode(CS, OUTPUT);    // CS (out from Arduino)
 pinMode(MOSI, OUTPUT);  // MOSI (data out from Arduino)
 pinMode(MISO, INPUT);   // MISO (data in to Arduino)
 pinMode(SCK, OUTPUT);   // SCK  (serial clock)
 digitalWrite(CS, HIGH);
} // end setup()

void loop() {
    byte OVL, OVH;      // overload condition HIGH and LOW, respectively
    unsigned int i;     // loop counter
    unsigned long w;
    long x;
    while (true) {
        digitalWrite(CS,HIGH);
        delayMicroseconds(100);
        digitalWrite(CS,LOW); // start next conversion
        delay(50);            // delay in milliseconds (nominal MCP3550-60 rate: 66.7 msec => 15 Hz)
        i=0;                  // use i as loop counter
        do {
            i++;                                              // use i for timeout
            delayMicroseconds(50);                            // loop keeps trying for up to 1 second
        } while ((digitalRead(MISO)==HIGH) && (i < 20000));   // wait for bit to drop low (ready)
        w = readword();       // data in:  32-bit word gets 24 bits via SPI port
        OVL = ((w & 0x800000) != 0x000000);  // ADC negative overflow bit (input > +Vref)
        OVH = ((w & 0x400000) != 0x000000);  // ADC positive overflow bit (input < -Vref)
        x = w << 2;  // to use the sign bit
        x = x/1024;  // to move the LSB to bit 0 position
        Serial.println(x);
    } // while (true)
} // end main loop()
