#include <Arduino.h>
#include <MCP335X.h>
#include <AM2320.h>
#include <MQ131.h>
#include <packets.h>
#include <uartcomm.h>

AM2320  thSensor(&Wire);
MCP335X adc(10, 11, 12, 13);
MQ131   mq131(MQ131Model::HighConcentration, &adc, &thSensor);

void setup(){
  Serial.begin(9600);
  adc.begin();
  thSensor.begin();
  mq131.begin();
  mq131.calibrate();
}

void loop(){
  adc.read();
  thSensor.read();
	mq131.read();
  sensorPacket.ozonePPM     = mq131.getO3();
  sensorPacket.ratio        = mq131.getRatio();
  sensorPacket.temperature  = thSensor.getTemperature();
  sensorPacket.humidity     = thSensor.getHumidity();
  send(PID_SENSOR, &sensorPacket, sizeof(SENSORPACK));
}
