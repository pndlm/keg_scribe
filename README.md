Keg Scribe
==========

Make your kegerator smarter -- track pours and report to an external HTTP service.

Spiritual successor to the [untappd kegerator](http://www.3d0g.net/brewing/untappd-kegerator), our goal is to build a basic flow meter system that reports results to an analytics engine.

It is an arduino project designed to run one or two flow meters. Our implementation uses a wifi shield with micro-SD for caching read data.

Finally, we use a networking library to connect to an HTTP service and transmit read data. Your implementation will vary at this point, because our backend is a custom PHP application.

Hardware
========

We developed against an [Arduino Uno](http://arduino.cc/en/Main/arduinoBoardUno).

We use the [CC3000 WiFi shield](http://www.adafruit.com/products/1491) to connect to our local Wifi network and write to a FAT16 formatted SD card.

We measure beer consumption with a [plastic flow meter from Adafruit](http://www.dx.com/p/yf-s201-hall-effect-water-flow-counter-sensor-black-217625#.U1Fi-uZdWHA). A second flow meter will be coming soon.

We measure temperature in and outside of the kegerator with a couple of temperature probes.

Dependencies
============

We make use of the following libraries. Make sure to install them in your Arduino Libraries folder before trying to compile:

Library|Purpose
|
[Adafruit CC3000 Library](https://github.com/adafruit/Adafruit_CC3000_Library)|Connect to Wifi network, send HTTP requests
[Fat16lib](https://code.google.com/p/fat16lib/)|Read and write to SD card with minimal memory requirements
[Time](http://www.pjrc.com/teensy/td_libs_Time.html)|Internal clock to generate time stamps for data points

Pinouts
=======

The default configuration is as follows:

Analog Pin|Connection
|
0|External (Ambient) Temperature Probe
1|Internal (Kegerator) Temperature Probe
2 through 5|Unused

Digital Pin|Connection
|
0|Unused
1|Unused
2|LED Anode to light up when pouring
3|CC3000 - IRQ
4|CC3000 - CS for SD Card
5|CC3000 - VBAT_EN
6|Kegerator Flow Sensor #1
7|Kegerator Flow Sensor #2 (coming soon)
10|CC3000 - SCK
11|CC3000 - CS for CC3000
12|CC3000 - MISO
13|CC3000 - MOSI

Authors and License
===================

Libraries and Flow Sensor code are copyright their respective authors. Other code is licensed under the MIT License. Use it how you want and don't sue us.
