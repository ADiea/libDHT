# libDHT
Arduino compatible DHT 11/22 lib. 
Based on Adafruit's lib https://github.com/adafruit/DHT-sensor-library
*Added dewpoint algorithms
*Added empiric confort function based on http://epb.apogee.net/res/refcomf.asp or docs/apogee_confort.png
*Add function that reads both temp and humidity in one pass
*Reorganised code and renamed member vars

**This was tested on an ARM architecture (ESP8266 module) running at 80Mhz
