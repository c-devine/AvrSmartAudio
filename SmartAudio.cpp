/*
 SmartAudio.cpp

 Created on: Oct 24, 2018
 Author: Chris
 */

#include "SmartAudio.h"

//
// CRC8 computations -  from betaflight
//

#define POLYGEN 0xd5
uint8_t CRC8(const uint8_t *data, const int8_t len) {
	uint8_t crc = 0; /* start with 0 so first byte can be 'xored' in */
	uint8_t currByte;

	for (int i = 0; i < len; i++) {
		currByte = data[i];

		crc ^= currByte; /* XOR-in the next input byte */

		for (int i = 0; i < 8; i++) {
			if ((crc & 0x80) != 0) {
				crc = (uint8_t) ((crc << 1) ^ POLYGEN);
			} else {
				crc <<= 1;
			}
		}
	}
	return crc;
}

SmartAudio::SmartAudio(uint8_t rxtx) :
		_ss(rxtx, rxtx, SA_TIMER, SA_BAUD, false) {

	_rxtx = rxtx;
	_rxBuffer = 0;
	_saState = SA_IDLE;

	memset(_buffer[0], 0, SA_RX_BUFFER_LEN);
	memset(_buffer[1], 0, SA_RX_BUFFER_LEN);
}

SmartAudio::~SmartAudio() {

	_ss.end();
}

void SmartAudio::setup() {
	_ss.begin();
}

void SmartAudio::getSettings() {

	static uint8_t buf[6] = { 0xAA, 0x55, 0x03, 0x00, 0x9F, 0x00 };

	sendFrame(buf, 6);

}

void SmartAudio::writeChannel(saWriteSettings_t settings) {

	uint8_t buf[7] = { 0xAA, 0x55, SACMD(SA_SET_CHANNEL), 0x01 };
	buf[4] = settings.channel;
	buf[5] = CRC8(buf, 5);
	buf[6] = 0x00;

	//{ 0xAA, 0x55, 0x07, 0x01 (length), 0x00 (channel), 0xB8, 0x00 };
	//printBuffer(buf, 7, "sending writeChannel to sa device");
	sendFrame(buf, 7);
}

void SmartAudio::writePower(saWriteSettings_t settings) {

	uint8_t buf[7] = { 0xAA, 0x55, SACMD(SA_SET_POWER), 0x01 };
	buf[4] = settings.power;
	buf[5] = CRC8(buf, 5);
	buf[6] = 0x00;

	//{ 0xAA, 0x55, 0x05, 0x01 (length), 0x00 (power), 0x6B(CRC8) }
	//printBuffer(buf, 7, "sending writePower to sa device");
	sendFrame(buf, 7);
}

void SmartAudio::writePitMode(saWriteSettings_t settings) {

	uint8_t buf[7] = { 0xAA, 0x55, SACMD(SA_SET_PITMODE), 0x01 };
	buf[4] = settings.pitMode ? SA_MODE_SET_IN_RANGE_PITMODE : SA_MODE_CLR_PITMODE ;
	buf[5] = CRC8(buf, 5);
	buf[6] = 0x00;

	//{0xAA 0x55 0x0B(Command 5) 0x01(Length) 0x01(IN RANGE PIT FLAG) 0xXX(CRC8)}
	//printBuffer(buf, 7, "sending writPitMode to sa device");
	sendFrame(buf, 7);

}

void SmartAudio::sendFrame(uint8_t *sendBuffer, uint8_t size) {

	uint8_t start = 0x0;
	_ss.stopListening();
	_ss.setTX();
	_ss.write(start);
	_ss.write(sendBuffer, size);

}

bool SmartAudio::processSmartAudio(uint32_t delayMs) {

	static uint8_t bufferPos = 0;

	if (!_ss.isListening()) {
		_ss.listen();
		//Serial.println("listening SA");
	}

	// docs say response comes back < 100ms
	delay(delayMs);

	while (_ss.available()) {
		uint8_t byte = _ss.read();

		switch (_saState) {

		case SA_IDLE:
			if (byte == SA_SYNC)
				_saState = SA_HEADER_START;
			break;

		case SA_HEADER_START:
			if (byte == SA_HEADER)
				_saState = SA_DATA_READ;
			else
				_saState = SA_IDLE;
			break;

		case SA_DATA_READ:
			_buffer[_rxBuffer][bufferPos] = byte;
			if (bufferPos > 0) {
				uint8_t size = _buffer[_rxBuffer][1];

				if (bufferPos == size + 2) {
					bufferPos = 0;
					_saState = SA_IDLE;
					_rxBuffer = !_rxBuffer;
					memset(_buffer[_rxBuffer], 0, SA_RX_BUFFER_LEN);
					return true;
				}
			}
			bufferPos++;
		}

	}

	return false;
}

void SmartAudio::printBuffer(uint8_t *buffer, uint8_t size, const char* msg) {
	Serial.print(msg);
	Serial.println("--------");
	for (uint8_t i = 0; i < size; i++) {
		Serial.print(buffer[i], HEX);
		Serial.print(" ");
	}
	Serial.println("\n--------");
}

uint8_t * SmartAudio::getBuffer() {
	return _buffer[!_rxBuffer];
}

