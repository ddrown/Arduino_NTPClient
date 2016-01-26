This is a NTP client for the ESP8266/Arduino platform.  It was originally based on the example NTP client bundled with the Arduino Time library, but is largely rewritten.

Example results with a local NTP server and a 47ppm drift corrected by manual configuration: https://dan.drown.org/esp8266/esp8266-ntp-with-static-47ppm.png

The purple errorbars are the uncertainty added to the offset by the round trip time.  The cyan line is to give a reference of what -0.259ppm looks like.
