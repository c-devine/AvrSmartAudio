/*
 * DotStartHelper.h
 *
 *  Created on: Nov 17, 2018
 *      Author: Chris
 */

#ifndef DOTSTARHELPER_H_
#define DOTSTARHELPER_H_

#include "Arduino.h"
#include <Adafruit_DotStar.h>

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(Adafruit_DotStar* dot, byte WheelPos) {
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return dot->Color(255 - WheelPos * 3, 0, WheelPos * 3);
	} else if (WheelPos < 170) {
		WheelPos -= 85;
		return dot->Color(0, WheelPos * 3, 255 - WheelPos * 3);
	} else {
		WheelPos -= 170;
		return dot->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
}

#endif /* DOTSTARHELPER_H_ */
