# libDHT
Arduino compatible DHT 11/22 lib with dewpoint and heat index functions. 

Based on Adafruit's lib https://github.com/adafruit/DHT-sensor-library with following modifications:
* Added dewpoint algorithms
* Added empiric comfort function based on http://epb.apogee.net/res/refcomf.asp or docs/apogee_confort.png
* Add function that reads both temp and humidity in one pass, maintain internal cache
* Reorganised code and renamed member vars, change code style etc

**This was tested on an ESP8266 module(ESP-12) running at 80Mhz. It is in theory compatible with Arduino but untested. **
If you own an Arduino and want to help with testing, open an issue.
