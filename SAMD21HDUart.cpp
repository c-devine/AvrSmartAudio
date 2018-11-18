/*
 Updates to SoftwareSerial.cpp - library for Arduino M0/M0 pro
 Copyright (c) 2016 Arduino Srl. All rights reserved.
 Written by Chiara Ruggeri (chiara@arduino.org)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 Enjoy!
 */

#include <Arduino.h>
#include "SAMDHDUart.h"
#include <variant.h>
#include <WInterrupts.h>

SAMDHDUart *SAMDHDUart::active_object = NULL;


// Constructor
SAMDHDUart::SAMDHDUart(uint8_t receivePin, uint8_t transmitPin, uint8_t timer,
		uint32_t baud, bool inverse_logic) :
		_timer(timer, &SAMDHDUart::timerISR, (float(1) / baud) * 1000000) {

	_receive_buffer_tail = 0;
	_receive_buffer_head = 0;

	_rx_delay_centering = 0;
	_rx_delay_intrabit = 0;
	_rx_delay_stopbit = 0;
	_tx_delay = 0;
	_buffer_overflow = false;
	_inverse_logic = inverse_logic;
	_bitCount = 0;
	_currentByte = 0;

	_receivePin = receivePin;
	_transmitPin = transmitPin;
	enableTimer(false);
	_baud = baud;

}

// Destructor
SAMDHDUart::~SAMDHDUart() {
	end();
}

bool SAMDHDUart::listen() {

	if (active_object != this) {
		if (active_object) {
			//Serial.print("stop listening on pin ");
			//Serial.println(active_object->_receivePin);
			active_object->stopListening();
		}

		_buffer_overflow = false;
		_receive_buffer_head = _receive_buffer_tail = 0;
		active_object = this;

		//Serial.print("listening to pin ");
		//Serial.println(active_object->_receivePin);

		setRX();

		if (_inverse_logic)
			//Start bit high
			attachInterrupt(_receivePin, handle_interrupt, RISING);
		else
			//Start bit low
			attachInterrupt(_receivePin, handle_interrupt, FALLING);

		return true;
	}
	return false;
}

bool SAMDHDUart::stopListening() {

	//Serial.println("stop listening");

	if (active_object == this) {
		detachInterrupt(_receivePin);
		active_object = NULL;
		return true;
	}
	return false;
}

inline void SAMDHDUart::timerISR(struct tc_module * const module_inst) {

	if (active_object)
		active_object->handleTimer();

}

void SAMDHDUart::enableTimer(bool enable) {
	//_timer.enable(enable);
	//_timer.enableInterrupt(enable);
	_timer.enableTimer(enable);
}

void SAMDHDUart::handleTimer() {

	if (rx_pin_read())
		_currentByte |= 1 << _bitCount;

	if (++_bitCount == 8) {

		enableTimer(false);

		// skip stop bit
		delayMicroseconds(_rx_delay_stopbit);

		if (_inverse_logic) {
			saveByte(~_currentByte);
			//Start bit high
			attachInterrupt(_receivePin, handle_interrupt, RISING);
		} else {
			saveByte(_currentByte);
			//Start bit low
			attachInterrupt(_receivePin, handle_interrupt, FALLING);
		}
	}

}

void SAMDHDUart::saveByte(uint8_t byte) {
	uint8_t next = (_receive_buffer_tail + 1) % _SS_MAX_RX_BUFF;
	if (next != _receive_buffer_head) {
		// save new data in buffer: tail points to where byte goes
		_receive_buffer[_receive_buffer_tail] = byte; // save new byte
		_receive_buffer_tail = next;
	} else {
		_buffer_overflow = true;
	}
}

void SAMDHDUart::recv() {

	// If RX line is high, then we don't see any start bit
	// so interrupt is probably not for us
	if (_inverse_logic ? rx_pin_read() : !rx_pin_read()) {

		detachInterrupt(_receivePin);
		// Wait approximately 1/2 of a bit width to "center" the sample
		delayMicroseconds(_rx_delay_centering);

		//detachInterrupt(_receivePin);
		_bitCount = 0;
		_currentByte = 0;
		enableTimer(true);

	}
}

uint32_t SAMDHDUart::rx_pin_read() {

	return digitalRead(_receivePin);
}

/* static */
inline void SAMDHDUart::handle_interrupt() {
	if (active_object) {
		active_object->recv();
	}
}

void SAMDHDUart::setTX() {

	digitalWrite(_transmitPin, _inverse_logic ? LOW : HIGH);
	pinMode(_transmitPin, OUTPUT);

}

void SAMDHDUart::setRX() {

	pinMode(_receivePin, INPUT);
	if (!_inverse_logic)
		digitalWrite(_receivePin, HIGH);  // pullup for normal logic!
	else
		digitalWrite(_receivePin, LOW);

}

void SAMDHDUart::begin() {

	// Precalculate the various delays
	//Calculate the distance between bit in micro seconds
	uint32_t bit_delay = (float(1) / _baud) * 1000000;

	_tx_delay = bit_delay - 4;

	// Only setup rx when we have a valid PCINT for this pin
	//if (digitalPinToInterrupt(_receivePin) != NOT_AN_INTERRUPT) {
	//Wait 1/2 bit - 2 micro seconds (time for interrupt to be served)
	// Updated for Trinket M0
	_rx_delay_centering = (bit_delay / 2) - 2;
	//_rx_delay_centering = (bit_delay / 2) - 2;
	//Wait 1 bit - 2 micro seconds (time in each loop iteration)
	// Updated for Trinket M0
	_rx_delay_intrabit = bit_delay - 2;
	//_rx_delay_intrabit = bit_delay - 2;
	//Wait 1 bit (the stop one)
	_rx_delay_stopbit = bit_delay;

	delayMicroseconds(_tx_delay);

	//}
	//listen();
}

void SAMDHDUart::end() {
	stopListening();
}

int SAMDHDUart::read() {
	if (!isListening()) {
		return -1;
	}

	// Empty buffer?
	if (_receive_buffer_head == _receive_buffer_tail) {
		return -1;
	}

	// Read from "head"
	uint8_t d = _receive_buffer[_receive_buffer_head]; // grab next byte
	_receive_buffer_head = (_receive_buffer_head + 1) % _SS_MAX_RX_BUFF;
	return d;
}

int SAMDHDUart::available() {
	if (!isListening())
		return 0;

	return (_receive_buffer_tail + _SS_MAX_RX_BUFF - _receive_buffer_head)
			% _SS_MAX_RX_BUFF;
}

size_t SAMDHDUart::write(uint8_t b) {

	if (_inverse_logic)
		b = ~b;

	// Write the start bit
	if (_inverse_logic)
		digitalWrite(_transmitPin, HIGH);
	else
		digitalWrite(_transmitPin, LOW);

	delayMicroseconds(_tx_delay);

	// Write each of the 8 bits
	for (uint8_t i = 8; i > 0; --i) {

		if (b & 1) // choose bit
			// send 1
			digitalWrite(_transmitPin, HIGH);
		else
			// send 0
			digitalWrite(_transmitPin, LOW);

		delayMicroseconds(_tx_delay);
		b >>= 1;
	}

	// restore pin to natural state
	if (_inverse_logic)
		digitalWrite(_transmitPin, LOW);
	else
		digitalWrite(_transmitPin, HIGH);

	delayMicroseconds(_tx_delay);

	return 1;
}

void SAMDHDUart::flush() {
	if (!isListening())
		return;

	_receive_buffer_head = _receive_buffer_tail = 0;

}

int SAMDHDUart::peek() {
	if (!isListening())
		return -1;

	// Empty buffer?
	if (_receive_buffer_head == _receive_buffer_tail)
		return -1;

	// Read from "head"
	return _receive_buffer[_receive_buffer_head];
}
