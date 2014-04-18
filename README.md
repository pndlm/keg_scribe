Keg Scribe
==========

Make your kegerator smarter -- track pours and report to an external HTTP service.

Spiritual successor to the [untappd kegerator](http://www.3d0g.net/brewing/untappd-kegerator), our goal is to build a basic flow meter system that reports results to an analytics engine.

It is an arduino project designed to run one or two flow meters. Our implementation uses a wifi shield with micro-SD for caching read data.

Finally, we use a networking library to connect to a RESTful service and transmit read data. Your implementation will vary at this point, because our backend is a custom PHP application.

Hardware
========

We developed against an [Arduino Uno](http://arduino.cc/en/Main/arduinoBoardUno)

Using two [plastic flow meters](http://www.dx.com/p/yf-s201-hall-effect-water-flow-counter-sensor-black-217625#.U1Fi-uZdWHA)

And the [CC3000 WiFi shield](http://www.adafruit.com/products/1491)

Pinouts
=======

Forthcoming
