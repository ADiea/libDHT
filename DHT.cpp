/*
 * Name: libDHT
 * License: MIT license. No warranty.
 * Location: https://github.com/ADiea/libDHT
 * Maintainer: ADiea (https://github.com/ADiea)
 *
 * Descr: Arduino compatible DHT 11/22 lib with dewpoint and heat index functions.
 *
 * Features:
 *	1. Read humidity and temperature in one function call.
 *	2. Determine heat index in *C or *F
 *	3. Determine dewpoint with various algorithms(speed vs accuracy)
 *	4. Determine thermal comfort
 *		4.1. Empiric comfort function based on comfort profiles
 *		4.2. Multiple comfort profiles. Default based on http://epb.apogee.net/res/refcomf.asp
 *		4.3. Determine if it's to cold, hot, humid, dry based on current comfort profile
 *
 * x/xx/xx -----:	TODO: Comfort profiles
 * 7/04/15 ADiea:	[experimental] comfort function; code reorganization
 * 7/02/15 ADiea:	dew point algorithms
 * 6/25/15 ADiea: 	pullup option; read temp and humidity in one function call
 *  	 	 	 	cache converted value for last temp and humid
 * 6/20/15 cloned from https://github.com/adafruit/DHT-sensor-library
 * -/--/-- written by Adafruit Industries
 */

#include "DHT.h"

void DHT::begin(void)
{
	//Pull the pin high to put the sensor in idle state
	pinMode(m_kSensorPin, OUTPUT);
	digitalWrite(m_kSensorPin, HIGH);

	//Make sure the first read() will happen
	m_lastreadtime = millis() - m_maxIntervalRead;

	//Delay 250ms at least before the first read, so the sensor sees a stable
	//pin HIGH output
	delay(250);
}

float DHT::readTemperature(bool bFarenheit/* = false*/)
{
	float f = NAN;
	readTempAndHumidity(&f, NULL, bFarenheit);
	return f;
}

float DHT::readHumidity(void)
{
	float f = NAN;
	readTempAndHumidity(NULL, &f);
	return f;
}

bool DHT::readTempAndHumidity(float* temp, float* humid, bool bFarenheit/* = false*/)
{
	bool bSuccess = false;

	if (read())
	{
		if(temp)
		{
			*temp = m_lastTemp;
			if(bFarenheit)
			{
				*temp = convertCtoF(*temp);
			}
		}
		if(humid)
		{
			*humid = m_lastHumid;
		}
		bSuccess = true;
	}
	return bSuccess;
}

float DHT::getHeatIndexF(float tempFahrenheit, float percentHumidity)
{
// Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
// Wikipedia: http://en.wikipedia.org/wiki/Heat_index
	float t2F = tempFahrenheit * tempFahrenheit;
	float h2 = percentHumidity * percentHumidity;

	return -42.379 + 2.04901523 * tempFahrenheit + 10.14333127 * percentHumidity
			+ -0.22475541 * tempFahrenheit * percentHumidity + -0.00683783 * t2F
			+ -0.05481717 * h2 + 0.00122874 * t2F * percentHumidity
			+ 0.00085282 * tempFahrenheit * h2 + -0.00000199 * t2F * h2;
}

float DHT::getHeatIndexC(float tempCelsius, float percentHumidity)
{
// Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
// Wikipedia: http://en.wikipedia.org/wiki/Heat_index
	float t2C = tempCelsius * tempCelsius;
	float h2 = percentHumidity * percentHumidity;

	return -8.784695 + 1.61139411 * tempCelsius + 2.33854900 * percentHumidity
			+ -0.14611605 * tempCelsius * percentHumidity + -0.01230809 * t2C
			+ -0.01642482 * h2 + 0.00221173 * t2C * percentHumidity
			+ 0.00072546 * tempCelsius * h2 + -0.00000358 * t2C * h2;
}

double DHT::getDewPoint(float tempCelsius, float percentHumidity, uint8_t algType /*= DEW_ACCURATE*/)
{
	double result = NAN;
	percentHumidity = percentHumidity * 0.01;

	switch(algType)
	{
		case DEW_ACCURATE: /* 6.040ms @ 80Mhz Accuracy +0.00 */

		{
			/* 01/JUL/2015 ADiea: ported from FORTRAN http://wahiduddin.net/calc/density_algorithms.htm */
			/*
			FUNCTION ESGG(T)
			Baker, Schlatter  17-MAY-1982     Original version.
			THIS FUNCTION RETURNS THE SATURATION VAPOR PRESSURE OVER LIQUID
			WATER ESGG (MILLIBARS) GIVEN THE TEMPERATURE T (CELSIUS). THE
			FORMULA USED, DUE TO GOFF AND GRATCH, APPEARS ON P. 350 OF THE
			SMITHSONIAN METEOROLOGICAL TABLES, SIXTH REVISED EDITION, 1963,
			BY ROLAND LIST.
			*/
			double CTA = 273.15,  // DIFFERENCE BETWEEN KELVIN AND CELSIUS TEMPERATURES
				   EWS = 3.00571489795, // log10 of SATURATION VAPOR PRESSURE (MB) OVER LIQUID WATER AT 100C
				   TS = 373.15; // BOILING POINT OF WATER (K)

			double  C1 = -7.90298, C2 = 5.02808, C3 = 1.3816E-7, C4 = 11.344, C5 = 8.1328E-3,  C6 = -3.49149;
			tempCelsius = tempCelsius + CTA;
			result = (TS / tempCelsius) - 1;

			//   GOFF-GRATCH FORMULA

			result = pow(10, (C1 * result + C2 * log10(result + 1) -
						  C3 * (pow(10, (C4 * (1. - tempCelsius / TS))) - 1.) +
						  C5 * (pow(10, (C6 * result)) - 1.) + EWS));
			if(result < 0)
				result = 0;
			//result now holds the saturation vapor pressure in mBar
			//	https://en.wikipedia.org/wiki/Vapor_pressure
			//Convert from mBar to kPa (1mBar = 0.1 kPa) and divide by 0.61078 constant
			//Determine vapor pressure (takes the RH into account)
			//	http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
			result = percentHumidity * result / (10 * 0.61078);
			result = log(result);
			result =(241.88 * result) / (17.558 - result);
		}
		break;

		case DEW_ACCURATE_FAST: /* 5.725ms @ 80Mhz Accuracy +0.01 */

		{
			/*Saturation vapor pressure is calculated by the datalogger
			 * with the following approximating polynomial
			 * (see Lowe, P.R. 1930. J. Appl. Meteor., 16:100-103):
			 * http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
			 */
			result = 6.107799961 +
						  tempCelsius * (0.4436518521 +
						  tempCelsius * (0.01428945805 +
						  tempCelsius * (2.650648471e-4 +
						  tempCelsius * (3.031240396e-6 +
						  tempCelsius * (2.034080948e-8 +
						  tempCelsius * 6.136820929e-11)))));
	        //Convert from mBar to kPa (1mBar = 0.1 kPa) and divide by 0.61078 constant
	        //Determine vapor pressure (takes the RH into account)
	        result = percentHumidity * result / (10 * 0.61078);
			result = log(result);
			result = (241.88 * result) / (17.558 - result);
		}
		break;

		case DEW_FAST: /* 1.479 ms Accuracy -0.04 */

		{
			/* 01/JUL/2015 ADiea: ported from FORTRAN http://wahiduddin.net/calc/density_algorithms.htm */
			/*
				Baker, Schlatter  17-MAY-1982     Original version.
				THIS FUNCTION RETURNS THE DEW POINT (CELSIUS) GIVEN THE TEMPERATURE
				(CELSIUS) AND RELATIVE HUMIDITY (%). THE FORMULA IS USED IN THE
				PROCESSING OF U.S. RAWINSONDE DATA AND IS REFERENCED IN PARRY, H.
				DEAN, 1969: "THE SEMIAUTOMATIC COMPUTATION OF RAWINSONDES,"
				TECHNICAL MEMORANDUM WBTM EDL 10, U.S. DEPARTMENT OF COMMERCE,
				ENVIRONMENTAL SCIENCE SERVICES ADMINISTRATION, WEATHER BUREAU,
				OFFICE OF SYSTEMS DEVELOPMENT, EQUIPMENT DEVELOPMENT LABORATORY,
				SILVER SPRING, MD (OCTOBER), PAGE 9 AND PAGE II-4, LINE 460.
			*/
				result = 1. - percentHumidity;

			/*  COMPUTE DEW POINT DEPRESSION. */
				result = (14.55 + 0.114 * tempCelsius)*result +
							 pow((2.5 + 0.007 * tempCelsius)*result, 3) +
							 (15.9 + 0.117 * tempCelsius)*pow(result, 14);

				result = tempCelsius - result;
		}
		break;

		case DEW_FASTEST: /* 1.415 ms @ 80Mhz Accuracy +0.10 */
		{
			/* http://en.wikipedia.org/wiki/Dew_point */
			double a = 17.271;
			double b = 237.7;
			result = (a * tempCelsius) / (b + tempCelsius) + log(percentHumidity);
			result = (b * result) / (a - result);
		}
		break;
	};

	return result;
}

float DHT::getComfortRatio(float tCelsius, float humidity, ComfortState& comfort)
{
	float ratio = 100; //100%
	float distance = 0;
	float kTempFactor = 3; //take into account the slope of the lines
	float kHumidFactor = 0.03; //take into account the slope of the lines
	uint8_t tempComfort = 0;
	
	comfort = Comfort_OK;

	distance = distanceTooHot(tCelsius, humidity);
	if(distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooHot;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kTempFactor;
	}
	
	distance = distanceTooHumid(tCelsius, humidity);
	if(distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooHumid;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kHumidFactor;
	}	
	
	distance = distanceTooCold(tCelsius, humidity);
	if(distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooCold;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kTempFactor;
	}

	distance = distanceTooDry(tCelsius, humidity);
	if(distance > 0)
	{
		//update the comfort descriptor
		tempComfort += (uint8_t)Comfort_TooDry;
		//decrease the comfot ratio taking the distance into account
		ratio -= distance * kHumidFactor;
	}

	comfort = (ComfortState)tempComfort;

	if(ratio < 0)
		ratio = 0;

	return ratio;
}

void DHT::updateInternalCache()
{
	/*Compute and write temp and humid to internal cache*/
	switch (m_kSensorType)
	{
		case DHT11:
			m_lastTemp = m_data[2];
			m_lastHumid = m_data[0];
			break;
		case DHT22:
		case DHT21:
			/*Temp*/
			m_lastTemp = ((uint32_t)(m_data[2] & 0x7F)<<8 | m_data[3]) / 10.0f;
			if (m_data[2] & 0x80)
			{
				m_lastTemp *= -1;
			}
			/*Humidity*/
			m_lastHumid = (((uint32_t)m_data[0])<<8 | m_data[1]) / 10.0f;
			break;
		default:
			Serial.println("libDHT: Sensor type not implemented");
			break;
	}
}

boolean DHT::read(void)
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;

	//Determine if it's appropiate to read the sensor, or return data from cache
	if ((millis() - m_lastreadtime) < m_maxIntervalRead)
	{
		return true; // will use last data from cache
	}
	m_lastreadtime = millis();

	m_data[0] = m_data[1] = m_data[2] = m_data[3] = m_data[4] = 0;

	//Pull the pin low for ~20 milliseconds
	pinMode(m_kSensorPin, OUTPUT);
	digitalWrite(m_kSensorPin, LOW);
	delay(20);
	//clear interrupts
	cli();
	//Make pin input and activate pullup
	pinMode(m_kSensorPin, INPUT);
	if (m_bPullupEnabled)
	{
		pullup(m_kSensorPin);
	}
	else
	{
		digitalWrite(m_kSensorPin, HIGH);
	}

	//Read in the transitions
	for (i = 0; i < MAXTIMINGS || j >= 40; i++)
	{
		counter = 0;
		while (digitalRead(m_kSensorPin) == laststate)
		{
			counter++;
			delayMicroseconds(1);
			if (counter == 255)
			{
				break;
			}
		}
		laststate = digitalRead(m_kSensorPin);

		if (counter == 255)
			break;

		// ignore first 3 transitions
		if ((i >= 4) && (i % 2 == 0))
		{
			// shove each bit into the storage bytes
			m_data[j / 8] <<= 1;

			if (counter > ONE_DURATION_THRESH_US)
			{
				m_data[j / 8] |= 1;
			}
			j++;
		}
	}
	sei();

#if DHT_DEBUG
	 Serial.println(j, DEC);
	 Serial.print(m_data[0], HEX); Serial.print(", ");
	 Serial.print(m_data[1], HEX); Serial.print(", ");
	 Serial.print(m_data[2], HEX); Serial.print(", ");
	 Serial.print(m_data[3], HEX); Serial.print(", ");
	 Serial.print(m_data[4], HEX); Serial.print(" =? ");
	 Serial.println((m_data[0] + m_data[1] + m_data[2] + m_data[3]) & 0xFF, HEX);
#endif

	// pull the pin high at the end
	 //(will stay high at least 250ms until the next reading)
	pinMode(m_kSensorPin, OUTPUT);
	digitalWrite(m_kSensorPin, HIGH);

	// check we read 40 bits and that the checksum matches
	if ((j >= 40) &&
	    (m_data[4] == ((m_data[0] + m_data[1] + m_data[2] + m_data[3]) & 0xFF)))
	{
		updateInternalCache();
		return true;
	}

	return false;
}
