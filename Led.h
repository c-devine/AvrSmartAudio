#ifndef LED_H_
#define LED_H_

#include "Arduino.h"

class Led {
public:
	Led(uint8_t pin);
	virtual ~Led();

	void toggle();
	void on();
	void off();
	void flash(uint16_t count, uint32_t delayMs);


private:
	uint8_t _pin;
	boolean _on;
};

#endif /* LED_H_ */
