/*
 * Name: libDHT
 * License: MIT license
 * Location: https://github.com/ADiea/libDHT
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

// Reference: http://epb.apogee.net/res/refcomf.asp
enum ComfortState
{
	Comfort_OK = 0,
	Comfort_TooHot = 1,
	Comfort_TooCold = 2,
	Comfort_TooDry = 4,
	Comfort_TooHumid = 8,
	Comfort_HotAndHumid = 9,
	Comfort_HotAndDry = 5,
	Comfort_ColdAndHumid = 10,
	Comfort_ColdAndDry = 6
};


class DHT
{
public:
	DHT(uint8_t pin, uint8_t type, boolean pullup = false, uint16_t maxIntervalRead = DSHEET_READ_INTERVAL)
		: m_kSensorPin(pin), m_kSensorType(type),
		  m_bPullupEnabled(pullup), m_firstRead(true), m_lastreadtime(0), m_maxIntervalRead(maxIntervalRead)
	{
		//In computing these constants the following reference was used http://epb.apogee.net/res/refcomf.asp
		//It was simplified as 4 straight lines and added very little skew on the vertical lines (+0.1 on x for C,D)
		//The for points used are(from top left, clock wise)
		//A(30%, 30*C) B(70%, 26.2*C) C(70.1%, 20.55*C) D(30.1%, 22.22*C)
		//On the X axis we have the rel humidity in % and on the Y axis the temperature in *C

		//Too hot line AB
		m_tooHot_m = -0.095;
		m_tooHot_b = 32.85;
		//Too humid line BC
		m_tooHumid_m =  -56.5;
		m_tooHumid_b = 3981.2;
		//Too cold line DC
		m_tooCold_m = -0.04175;
		m_tooHCold_b = 23.476675;
		//Too dry line AD
		m_tooDry_m = -77.8;
		m_tooDry_b = 2364;
	};

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

	float comfortRatio(ComfortState* comfort);

	inline ComfortState isTooHot(float tCelsius, float humidity)
		{return (tCelsius > (humidity * m_tooHot_m + m_tooHot_b)) ? Comfort_TooHot : Comfort_OK;}
	inline ComfortState isTooHumid(float tCelsius, float humidity)
		{return (tCelsius > (humidity * m_tooHumid_m + m_tooHumid_b)) ? Comfort_TooHumid : Comfort_OK;}
	inline ComfortState isTooCold(float tCelsius, float humidity)
		{return (tCelsius < (humidity * m_tooCold_m + m_tooHCold_b)) ? Comfort_TooCold : Comfort_OK;}
	inline ComfortState isTooDry(float tCelsius, float humidity)
		{return (tCelsius < (humidity * m_tooDry_m + m_tooDry_b)) ? Comfort_TooDry : Comfort_OK;}



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

	//Represent the 4 lines dry, humid, hot, cold, using the y = mx + b formula
	float m_tooHot_m, m_tooHot_b;
	float m_tooCold_m, m_tooHCold_b;
	float m_tooDry_m, m_tooDry_b;
	float m_tooHumid_m, m_tooHumid_b;

	float m_lastTemp, m_lastHumid;
};
#endif
