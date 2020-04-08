/******************************************************************************
 * Arduino-MQ131-driver                                                       *
 * --------------------                                                       *
 * Arduino driver for gas sensor MQ131 (O3)                                   *
 * Author: Olivier Staquet                                                    *
 * Modified by: NowTechnologies Ltd to use with Ozone 2Click board            *
 *               + external temperature & humidity sensor                     *
 * Original version available                                                 *
 *  at https://github.com/ostaquet/Arduino-MQ131-driver                       *
 ******************************************************************************/

#include "MQ131.h"

/**
 * Constructor, nothing special to do
 */
MQ131::MQ131(MQ131Model _model, MCP335X* mcp335x_ptr, AM2320* tempHumidity_ptr, bool heaterControl) {
	controlHeater = heaterControl;
	adc = mcp335x_ptr;
	model = _model;
	th = tempHumidity_ptr;
	switch (model) {
	case MQ131Model::LowConcentration :
	  setRL(MQ131_DEFAULT_LO_CONCENTRATION_RL);
	  setR0(MQ131_DEFAULT_LO_CONCENTRATION_R0);
	  setTimeToRead(MQ131_DEFAULT_LO_CONCENTRATION_TIME2READ);
	  break;
	case MQ131Model::HighConcentration :
	  setRL(MQ131_DEFAULT_HI_CONCENTRATION_RL);
	  setR0(MQ131_DEFAULT_HI_CONCENTRATION_R0);
	  setTimeToRead(MQ131_DEFAULT_HI_CONCENTRATION_TIME2READ);
	  break;
	}
}

/**
 * Destructor, nothing special to do
 */
MQ131::~MQ131() {
}

/**
 * Init core variables
 */
void MQ131::begin(int _pinHeater, int _pinSensor) {

	// Check pins
	if (adc == NULL && _pinSensor == -1) {
		Serial.println("ERROR: supplying sensor pin (analog input) is necessary if MCP335X is not used!");
	}
	if (controlHeater == true && _pinHeater == -1) {
		Serial.println("ERROR: supplying heater pin (digital output) is necessary if sensor control is used!");
	}
	if (th == NULL) {
		Serial.println("INFO: use setEnv(temp, humidity) before measurements to get accurate results!");
	}

	// Store the circuit info (pin and load resistance)
	pinHeater = _pinHeater;
	pinSensor = _pinSensor;

	// Setup pin modes
	if (controlHeater) {
		pinMode(pinHeater, OUTPUT);
		// Switch off the heater as default status
		digitalWrite(pinHeater, LOW);
		heaterOn = false;
	} else {
		// Heater is controlled externally, assuming it is ALWAYS ON!
		heaterOn = true;
	}
	if (adc == NULL) {
		pinMode(pinSensor, INPUT);
	}
}

/**
 * Do a full cycle (heater, reading, stop heater)
 * The function blocks the thread only until the end of the read cycle!
 */
void MQ131::read() {
	startHeater();
	while(!isTimeToRead()) {
#ifdef MQ131_VERBOSE
		Serial.println("Heater Control: waiting to heat up...");
		delay(500); // 0.5sec wait
#endif
	}
	lastValueRs = readRs();
#ifdef MQ131_VERBOSE
	Serial.print("Rs = "); Serial.println(lastValueRs);
#endif
	if (th != NULL) {
		int temp = th->getTemperature();
		int hum  = th->getHumidity();
		if (temp == 0 && hum == 0) {
			// Sensor error or sensor disconnected
			setEnv(MQ131_DEFAULT_TEMPERATURE_CELSIUS, MQ131_DEFAULT_HUMIDITY_PERCENT);
		} else {
			setEnv(temp, hum);
		}
	}
	stopHeater();
}

/**
 * Start the heater
 */
void MQ131::startHeater() {
	if (controlHeater) {
		digitalWrite(pinHeater, HIGH);
		secLastStart = millis()/1000;
		heaterOn = true;
	}
}

/**
 * Check if it is the right time to read the Rs value
 */
bool MQ131::isTimeToRead() {
	// Check if the heater has been started...
	if (!heaterOn) {
		return false;
	}
	if (controlHeater) {
		// OK, check if it's the time to read based on calibration parameters
		if (millis()/1000 >= secLastStart + getTimeToRead()) {
			return true;
		}
		return false;
	} else {
		return true; // Assume that heater has been ON for long enough to read correct value
	}
}

/**
 * Stop the heater
 */
void MQ131::stopHeater() {
	if (controlHeater) {
		digitalWrite(pinHeater, LOW);
		secLastStart = 0;
		heaterOn = false;
	}
}

/**
 * Get parameter time to read
 */
long MQ131::getTimeToRead() {
	return secToRead;
}

/**
 * Set parameter time to read (for calibration or to recall
 * calibration from previous run)
 */
void MQ131::setTimeToRead(long sec) {
	secToRead = sec;
}

/**
 * Read Rs value
 */
float MQ131::readRs() {
	float vRL = 0.0;
	if (adc == NULL) {
		// Read the value
		int valueSensor = analogRead(pinSensor);
		// Compute the voltage on load resistance (for 5V Arduino)
		vRL = ((float)valueSensor) / 1024.0 * 5.0;
	} else {
		long valueSensor = adc->read(); // 22-bit ADC +/- 2097152 range
		vRL = ((float)valueSensor) / 2097152.0 * 5.0;
	}
	// Compute the resistance of the sensor (for 5V Arduino)
	float rS = (5.0 / vRL - 1.0) * valueRL;
	return rS;
}

/**
 * Set environmental values
 */
void MQ131::setEnv(int tempCels, int humPc) {
	temperatureCelsius = tempCels;
	humidityPercent = humPc;
#ifdef MQ131_VERBOSE
	Serial.print("Temperature: "); Serial.print(temperatureCelsius); Serial.print(" [C]");
	Serial.print(", Humidity: "); Serial.print(humidityPercent); Serial.println(" [%]");
#endif
}

/**
 * Get correction to apply on Rs depending on environmental
 * conditions
 */
float MQ131::getEnvCorrectRatio() {
	// Select the right equation based on humidity
	// If default value, ignore correction ratio
	if (humidityPercent == 60 && temperatureCelsius == 20) {
		return 1.0;
	}
	// For humidity > 75%, use the 85% curve
	if (humidityPercent > 75) {
		// R^2 = 0.9986
		return -0.0141 * temperatureCelsius + 1.5623;
	}
	// For humidity > 50%, use the 60% curve
	if (humidityPercent > 50) {
		// R^2 = 0.9976
		return -0.0119 * temperatureCelsius + 1.3261;
	}
	// Humidity < 50%, use the 30% curve
	// R^2 = 0.996
	return -0.0103 * temperatureCelsius + 1.1507;
 }

/**
 * Get gas concentration for O3 in OzoneUnit::PPM
 */
float MQ131::getO3(OzoneUnit unit) {
	// If no value Rs read, return 0.0
	if (lastValueRs < 0) {
#ifdef MQ131_VERBOSE
		Serial.println("Calibration or reading was not performed, use read() or calibrate()");
#endif
		return 0.0;
	}
	ratio = lastValueRs / valueR0 * getEnvCorrectRatio();
#ifdef MQ131_VERBOSE
		Serial.print("RS=");Serial.println(lastValueRs);
		Serial.print("R0=");Serial.println(valueR0);
		Serial.print("r =");Serial.println(ratio);
#endif
	switch (model) {
		case MQ131Model::LowConcentration :
			// Use the equation to compute the O3 concentration in OzoneUnit::PPM
			// R^2 = 0.9987
			// Compute the ratio Rs/R0 and apply the environmental correction
			return convert(9.4783 * pow(ratio, 2.3348), OzoneUnit::PPB, unit);
		case MQ131Model::HighConcentration :
			// Use the equation to compute the O3 concentration in OzoneUnit::PPM
			// R^2 = 0.99
			// Compute the ratio Rs/R0 and apply the environmental correction
			return convert(8.1399 * pow(ratio, 2.3297), OzoneUnit::PPM, unit);
		default :
			return 0.0;
	}
}

/**
 * Convert gas unit of gas concentration
 */
float MQ131::convert(float input, OzoneUnit unitIn, OzoneUnit unitOut) {
	if (unitIn == unitOut) {
		return input;
	}
	float concentration = 0;
	switch(unitOut) {
		case OzoneUnit::PPM :
		  // We assume that the unit IN is OzoneUnit::PPB as the sensor provide only in OzoneUnit::PPB and OzoneUnit::PPM
		  // depending on the type of sensor (METAL or BLACK_BAKELITE)
		  // So, convert OzoneUnit::PPB to OzoneUnit::PPM
		  return input / 1000.0;
		case OzoneUnit::PPB :
		  // We assume that the unit IN is OzoneUnit::PPM as the sensor provide only in OzoneUnit::PPB and OzoneUnit::PPM
		  // depending on the type of sensor (METAL or BLACK_BAKELITE)
		  // So, convert OzoneUnit::PPM to OzoneUnit::PPB
		  return input * 1000.0;
		case OzoneUnit::MG_M3:
		  if (unitIn == OzoneUnit::PPM) {
			concentration = input; // PPM
		  } else {
			concentration = input / 1000.0; // OzoneUnit::PPB to PPM
		  }
		  return concentration * 48.0 / 22.71108; // PPM to mg/m3
		case OzoneUnit::UG_M3:
		  if (unitIn == OzoneUnit::PPB) {
			concentration = input; // OzoneUnit::PPB
		  } else {
			concentration = input * 1000.0; // PPM to OzoneUnit::PPB
		  }
		  return concentration * 48.0 / 22.71108; // OzoneUnit::PPB to ug/m3
		default:
		  return input;
	}
}

/**
 * Calibrate the basic values (R0 and time to read)
 */
void MQ131::calibrate() {
	// Take care of the last Rs value read on the sensor (forget the decimals)
	float lastRsValue = 0;
	// Count how many time we keep the same Rs value in a row
	int countReadInRow = 0;
	// Count how long we have to wait to have consistent value
	int count = 0;
	// Start heater
	startHeater();
	int timeToReadConsistency = MQ131_DEFAULT_STABLE_CYCLE;
	if (adc == NULL) {
		// Low-res analog input, read multiple values until readings settle
		while(countReadInRow <= timeToReadConsistency) {
			float value = readRs();
			if((int)lastRsValue != (int)value) {
				lastRsValue = value;
				countReadInRow = 0;
			} else {
				countReadInRow++;
			}
			count++;
			delay(1000);
		}
	} else {
		// High-res ADC, read RS
		lastRsValue = readRs();
	}
	// Stop heater
	stopHeater();
	// We have our R0 and our time to read
	if (controlHeater) {
		setR0(lastRsValue);
		setTimeToRead(count);
	} else {
		// Keep R0 from specs
	}
}

/**
 * Store R0 value (come from calibration or set by user)
 */
void MQ131::setR0(float _valueR0) {
	valueR0 = _valueR0;
}

/**
 * Store RL value (set by user)
 */
void MQ131::setRL(long _valueRL) {
	valueRL = _valueRL;
}

/**
 * Get R0 value
 */
float MQ131::getR0() {
	return valueR0;
}

/**
 * Get ratio
 */
float MQ131::getRatio(){
	return ratio;
}
