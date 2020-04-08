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

#ifndef _MQ131_H_
#define _MQ131_H_

#include <Arduino.h>
#include <MCP335X.h>
#include <AM2320.h>

// #define MQ131_VERBOSE

// Default values

#define MQ131_DEFAULT_STABLE_CYCLE                20             // Number of cycles with low deviation to consider the calibration as stable and reliable
#define MQ131_DEFAULT_TEMPERATURE_CELSIUS         20             // Default temperature to correct environmental drift
#define MQ131_DEFAULT_HUMIDITY_PERCENT            65             // Default humidity to correct environmental drift
#define MQ131_DEFAULT_LO_CONCENTRATION_RL         10000          // Default load resistance of 10 kOhms
#define MQ131_DEFAULT_LO_CONCENTRATION_R0         110470.60      // Default R0 for low concentration MQ131
#define MQ131_DEFAULT_LO_CONCENTRATION_TIME2READ  72             // Default time to read before stable signal for low concentration MQ131
#define MQ131_DEFAULT_HI_CONCENTRATION_RL         10000          // Default load resistance of 10 kOhms
#define MQ131_DEFAULT_HI_CONCENTRATION_R0         385.40         // Default R0 for high concentration MQ131
#define MQ131_DEFAULT_HI_CONCENTRATION_TIME2READ  80             // Default time to read before stable signal for high concentration MQ131

enum class MQ131Model {
	LowConcentration,
	HighConcentration
};

enum class OzoneUnit {
	PPM, PPB, MG_M3, UG_M3
};

class MQ131 {
private:
	// Internal helpers
	void startHeater(); /// Internal function to manage the heater
	bool isTimeToRead();
	void stopHeater();
	float readRs(); /// Internal reading function of Rs
	float getEnvCorrectRatio(); /// Get environmental correction to apply on ration Rs/R0
	float convert(float input, OzoneUnit unitIn, OzoneUnit unitOut); /// Convert gas unit of gas concentration
	// Internal variables
	MQ131Model model; /// Model of MQ131
	MCP335X* adc;       /// External MCP335X 22-bit ADC for Ozone 2Click boards
	AM2320*  th;        /// External AM2320 temperature and humidity sensor
	bool heaterOn;      /// True if heater is on, false otherwise
	bool controlHeater; /// True if this driver controls heater, false if heater is controlled externally (always on!)
	int pinHeater = -1; /// Heater pin
	int pinSensor = -1; /// Sensor pin
	long valueRL = -1; /// load resistance value
	// Timer to keep track of the pre-heating
	unsigned long secLastStart = 0;
	long secToRead = -1;
	float valueR0 = -1; /// Calibration of R0
	float lastValueRs = -1; /// Last value for sensor resistance
	float ratio = -1;
	// Parameters for environment
	int temperatureCelsius = MQ131_DEFAULT_TEMPERATURE_CELSIUS; /// Environmental temperature in C
	int humidityPercent = MQ131_DEFAULT_HUMIDITY_PERCENT; /// Environmental humidity in %
public:
	MQ131(MQ131Model _model, MCP335X* mcp335x_ptr = NULL, AM2320* tempHumidity_ptr = NULL, bool heaterControl = false);
	virtual ~MQ131();
	// Initialize the driver, this will NOT call begin on MCP335X, neither on AM2320
	void begin(int _pinHeater = -1, int _pinSensor = -1);
	// Manage a full cycle with delay() if heater control is used
	void read();
	// Read the concentration of gas
	// The environment should be set for accurate results
	float getO3(OzoneUnit unit = OzoneUnit::PPM);
	// Define environment
	// Define the temperature (in Celsius) and humidity (in %) to adjust the
	// output values based on typical characteristics of the MQ131
	void setEnv(int tempCels, int humPc);
	// Setup calibration: Time to read
	// Define the time to read after started the heater
	// Get function also available to know the value after calibrate()
	// (the time to read is calculated automatically after calibration)
	void setTimeToRead(long sec);
	long getTimeToRead();
	// Setup calibration: R0
	// Define the R0 for the calibration
	// Get function also available to know the value after calibrate()
	// (the time to read is calculated automatically after calibration)
	void setR0(float _valueR0);
	// Setup calibration: RL Load resistance
	void setRL(long _valueRL);
	float getR0();
	float getRatio();
	// Launch full calibration cycle
	// Ideally, 20°C 65% humidity in clean fresh air (can take some minutes)
	// For further use of calibration values, please use getTimeToRead() and getR0()
	void calibrate();
};

#endif // _MQ131_H_
