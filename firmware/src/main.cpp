#include <Arduino.h>
#include <MCP335X.h>
#include <AM2320.h>

AM2320  thSensor(&Wire);
MCP335X ozoneSensor(10, 11, 12, 13);
boolean thSenseFound = false;
boolean ozSenseFound = false;

void setup(){
  Serial.begin(115200);
  ozoneSensor.begin();
  thSensor.begin();
}

void loop(){
    ozoneSensor.read();
    thSensor.read();
    Serial.print("O3: "); Serial.println(ozoneSensor.getLastValue());
    Serial.print("TC: "); Serial.println(thSensor.getTemperature());
    Serial.print("RH: "); Serial.println(thSensor.getHumidity());
}
