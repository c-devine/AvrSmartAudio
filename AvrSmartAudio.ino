#include <Adafruit_DotStar.h>
#include <avdweb_SAMDtimer.h>
#include "Arduino.h"
#include "FrskySPort.h"
#include "Vtx.h"
#include "SmartAudio.h"
#include "DotStarHelper.h"

#define LED_PIN 13
#define SERIAL_BAUD 9600

#define SPORT_PIN 3
#define SA_PIN 2

Adafruit_DotStar dot = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK,
DOTSTAR_BGR);

FrskySPort frsport(SPORT_PIN);
SmartAudio sa(SA_PIN);
Vtx vtx(frsport, sa);

void toggleLed() {
	digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void setup() {

	Serial.begin(SERIAL_BAUD);
	dot.begin();
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);
	vtx.setup();

}

void loop() {

	static uint8_t ledCounter = 0;
	static uint32_t millisCounter = millis();
	static uint32_t dotCounter = millis();

	if (millis() - millisCounter > 1000) {
		toggleLed();
		millisCounter = millis();
		Serial.println(".");
	}

	if (millis() - dotCounter > 25) {
		uint32_t color = Wheel(&dot, ledCounter++);
		dot.setPixelColor(0, color);
		dot.show();
		dotCounter = millis();
	}

	vtx.loop();

}
