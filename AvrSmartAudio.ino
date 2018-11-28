#include <stdlib.h>
#include <Adafruit_DotStar.h>
#include <avdweb_SAMDtimer.h>
#include "Arduino.h"
#include "FrskySPort.h"
#include "Vtx.h"
#include "SmartAudio.h"
#include "Gps.h"
#include "Led.h"
#include "DotStarHelper.h"

#define SERIAL_BAUD 9600
#define LED_PIN 13
#define SPORT_PIN 1
#define SA_PIN 2

FrskySPort frsport(SPORT_PIN);
SmartAudio sa(SA_PIN);
Vtx vtx(frsport, sa);
Gps gps(Serial1, false);
Led led(LED_PIN);
Adafruit_DotStar dot = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK,
DOTSTAR_BGR);

void setup() {

	Serial.begin(SERIAL_BAUD);
	dot.begin();
	vtx.setup();
	gps.setup();
}

void loop() {

	static bool gpsFix = false;
	static uint8_t ledCounter = 0;
	static uint32_t millisCounter = millis();
	static uint32_t dotCounter = millis();

	if (millis() - millisCounter > 1000) {
		// solid led if there is a gps fix, otherwise flash
		if (!gpsFix) {
			led.toggle();
		} else {
			led.on();
		}
		millisCounter = millis();
		Serial.println(".");
	}

	// need to figure out a way to tell if SPort is connected.
	if (millis() - dotCounter > 25) {
		uint32_t color = Wheel(&dot, ledCounter++);
		dot.setPixelColor(0, color);
		dot.show();
		dotCounter = millis();
	}

	vtx.loop();

	geodata_t* geoData = gps.processSerial();
	if (geoData) {
		if (geoData->gpsFix) {
			gpsFix = true;
			frsport.sendLatLon(geoData);
		}
	}

}
