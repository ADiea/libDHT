/*
 * Name: libDHT
 * License: MIT license
 *
 * 7/02/15 ADiea:	dew point algorithms
 * 6/25/15 ADiea: 	pullup option
 *  	 	 	 	heat index functions
 *  	 	 	 	read temp and humidity in one function call
 *  	 	 	 	cache converted value for last temp and humid
 * 6/20/15 cloned from https://github.com/adafruit/DHT-sensor-library
 * -/-/-	written by Adafruit Industries
 */

#ifndef DHT_H
#define DHT_H
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define DHT_DEBUG 0 //Change to 1 for debug output

#define ONE_DURATION_THRESH_US 30 //From datasheet: '0' if HIGH lasts 26-28us,
								  //				'1' if HIGH lasts 70us

// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85

#define DHT11 11
#define DHT22 22
#define DHT21 21
#define AM2301 21

#define DSHEET_READ_INTERVAL 2000
#define LONG_READ_INTERVAL 4000

#define DEW_ACCURATE 0
#define DEW_FAST 1
#define DEW_ACCURATE_FAST 2
#define DEW_FASTEST 3


class DHT
{
public:
	DHT(uint8_t pin, uint8_t type, boolean pullup = false, uint16_t maxIntervalRead = DSHEET_READ_INTERVAL)
		: m_kSensorPin(pin), m_kSensorType(type),
		  m_bPullupEnabled(pullup), m_firstRead(true), m_lastreadtime(0), m_maxIntervalRead(maxIntervalRead){};

	void begin(void);

	float readSensor(bool bReadTemperature, bool bFarenheit = false);
	float readTemperature(bool bFarenheit = false);
	float readHumidity(void);

	bool readTempAndHumidity(float* temp, float* humid, bool bFarenheit = false);

	float convertCtoF(float);
	float convertFtoC(float);
	float computeHeatIndexC(float tempCelsius, float percentHumidity); //TODO:test accuracy against computeHeatIndexF
	float computeHeatIndexF(float tempFahrenheit, float percentHumidity);
	double computeDewPoint(float tempCelsius, float percentHumidity, uint8_t algType = DEW_ACCURATE);
private:
	boolean read(void);
	void updateInternalCache();

	boolean m_firstRead;
	uint8_t m_kSensorPin, m_kSensorType;
	uint8_t m_data[6];
	unsigned long m_lastreadtime;
	boolean m_bPullupEnabled;

	//The datasheet advises to read no more than one every 2 seconds.
	//However if reads are done at greater intervals the sensor's output will be less subject to self-heating
	//Reference: http://www.kandrsmith.org/RJS/Misc/dht_sht_how_fast.html
	uint16_t m_maxIntervalRead;

	float m_lastTemp, m_lastHumid;
};
#endif
