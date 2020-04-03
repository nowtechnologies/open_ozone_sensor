#ifndef AM2320_H
#define AM2320_H

#include <Arduino.h>
#include <Wire.h>

#define AM2320_address (0xB8 >> 1)

class AM2320
{
  private:
    TwoWire* communication;
    float cTemp;
    float Humidity;
  public:
    AM2320(TwoWire* com_wire);
    uint8_t State;
    void begin(void);
    uint8_t read(void);
    uint8_t startConvert(void);
    uint8_t getData(void);
    float getHumidity(void);
    float getTemperature(void);
};

#endif
