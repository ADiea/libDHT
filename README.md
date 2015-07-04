# libDHT
Arduino compatible DHT 11/22 lib with dewpoint and heat index functions. 

Features:
1. Read humidity and temperature in one function call.
2. Determine heat index in *C or *F
3. Determine dewpoint with various algorithms(speed vs accuracy)
4. Determine thermal comfort
	4.1. Empiric comfort function based on comfort profiles
	4.2. Multiple comfort profiles. Default based on http://epb.apogee.net/res/refcomf.asp
	4.3. Determine if it's to cold, hot, humid, dry based on current comfort profile
5. Optimized for sensor read speed(), stack and code size.

Based on *Adafruit's lib* https://github.com/adafruit/DHT-sensor-library

**This was tested on an ESP8266 module(ESP-12) running at 80Mhz. _It is in theory compatible with Arduino but untested..._**

If you own an Arduino and want to help with testing, open an issue and let me know.
