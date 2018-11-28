# AvrSmartAudio

Controller firmware used to control a Smart Audio enabled VTX with the existing betaflight OpenTX lua scripts. The [Adafruit Trinket M0](https://www.adafruit.com/product/3500) is a small SAMD21 based development board with USB (easy programming), multiple GPIO pins, and a couple of leds. The Trinket is used to receive the S.Port telemetry, create read/write settings messages for the Smart Audio VTX, and return the results to the radio. This project was created to enable Smart Audio control on aircraft _without_ a flight controller board running something like betafight - but could be extended to control the other pins on the Trinket, or control some other device, or capture GPS... Tested soley with a Mach 2 VTX.

## Overview

<img src="https://raw.githubusercontent.com/c-devine/AvrSmartAudio/snapshots/assets/img/overview.png?raw=true">


## Video
Smart Audio in action:

[![Demo](https://raw.githubusercontent.com/c-devine/AvrSmartAudio/snapshots/assets/img/youtube-prototype.png?raw=true)](https://www.youtube.com/watch?v=tcKi-m7yl1k "Adafruit Trinket Smart Audio Prototype on Breadboard")


## To Do
* Clean up code
* Add auto-bauding
* Make buildable in Arduino IDE
* Update leds based on status (connected, disconnected, etc).

## Snapshots


## Hardware


## Software 
Uses a hacked version of "SoftwareSerial".  Updated start/stop, intra-bit delays, and timings that worked on the SAMD21. Also changed the RX read strategy to use a Timer and timed pin reads, instead of delays - and removed the shared receive buffer.  The SAMD21HDUart should only listen to one RX at a time.


## Prerequisites
* [Avdweb_SAMDtimer](http://www.avdweb.nl/arduino/libraries/samd21-timer.html) - SAMD21 Timer library - requires two other specific libraries (see readme in the referenced link):  Adafruit\_ASFcore and Adafruit_ZeroTimer
* [Adafruit Dot Star Library](https://github.com/adafruit/Adafruit_DotStar) - Library used to control the on-board led.


## Building and Installing
Clone and build in eclipse. Use the "Upload Sketch" tool in Sloeber IDE, or possibly use the Arduino IDE (but the code setup probably needs to be re-arranged.) 

## Built With

* [Sloeber Eclipse Plugin](https://github.com/Sloeber/arduino-eclipse-plugin) - Eclipse based IDE

## Contributing

Submit a pull request if interested in contributing.

## Versioning

[SemVer](http://semver.org/) for versioning.

## Authors

* **Chris** - *Initial work* - [c-devine](https://github.com/c-devine)


## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* [betaflight](https://github.com/betaflight/betaflight) - Open Source Flight Controller Firmware.
* [Circular Buffer](https://github.com/rlogiacco/CircularBuffer) - Circular Buffer code.
