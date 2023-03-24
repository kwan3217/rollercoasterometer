# Shipometer
The *Shipometer* is designed to record as much data as possible on a cruise
ship. To this end, it is composed of a Raspberry Pi running Raspbian, along
with several other programs, and both off-the-shelf and custom hardware.

Sensors include:

* ADS-B radio for recording flight information
* AIS radio for recording ship logs
* GPS for own position and timing
* Inertial measurement unit because we can
* Pressure, temperature, and humidity sensor because it's interesting
* Magnetometer, which will be challenging to use on a steel ship
* Logging of on-chip die temperature, clock frequency, and load.

This folder documents the Shipometer HAT, which holds a socket for a Sparkfun
ZED-F9R breakout GPS. This part has its own inertial sensor and GPS receiver,
as well as a serial data stream and an accurate PPS signal.

We also include sockets for what I call *stamps* (based on their appearance
with castellated edges). The stamps take up much less space than a full breakout
board, but at the same time can be independently assembled and tested. I have
in mind the wild idea of a whole series of stamps, all with roughly compatible
pinouts. So far I have designed two, one which supports a BME280 
pressure/temperature/humidity sensor, and one which supports an ICM20948 9DoF
sensor.

The hat itself includes an LPC2102 microcontroller, used as a timer. It is able
to record the time of each PPS to within one 60MHz clock cycle, and is also
hooked up to time the data-ready outputs of the inertial and pressure stamp.

It also includes hardware to multiplex the chip select lines (so that all chips
can be read in SPI mode) and a 1.8V regulator, both to supply power to the timer
and to the inertial sensor.

