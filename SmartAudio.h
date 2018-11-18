/*
 * SmartAudio.h
 *
 *  Created on: Oct 24, 2018
 *      Author: Chris
 */

#ifndef SMARTAUDIO_H_
#define SMARTAUDIO_H_

#include "Arduino.h"
#include "SAMDHDUart.h"

#define SA_GET_SETTINGS 0x01
#define SA_SET_POWER 0x02
#define SA_SET_CHANNEL 0x03
#define SA_SET_FREQUENCY 0x04
#define SA_SET_PITMODE 0x05

#define SA_OPMODE_SETFREQ 0
#define SA_OPMODE_PITMODE_ACTIVE 1
#define SA_OPMODE_IN_RANGE_PITMODE 2
#define SA_OPMODE_OUT_RANGE_PITMODE 3
#define SA_OPMODE_UNLOCKED 4


#define SA_MODE_SET_IN_RANGE_PITMODE 0x01
#define SA_MODE_SET_OUT_RANGE_PITMODE 0x02
#define SA_MODE_CLR_PITMODE 0x04
#define SA_MODE_SET_UNLOCK  0x08
#define SA_MODE_SET_LOCK 0x00

#define SA_TIMER 4
#define SA_BAUD 4800
#define SA_SYNC 0xAA
#define SA_HEADER 0x55

#define MSP_SA_PROTOCOL 3

#define SA_RX_BUFFER_LEN 32

#define SACMD(cmd) (((cmd) << 1) | 1)

#define SA_DEVICE_CHVAL_TO_BAND(val) ((val) / (uint8_t)8)
#define SA_DEVICE_CHVAL_TO_CHANNEL(val) ((val) % (uint8_t)8)

typedef enum {
	SA_IDLE, SA_HEADER_START, SA_DATA_READ
} saState_e;

typedef struct saWriteSettings_s {
	int8_t channel;
	int8_t pitMode;
	int8_t power;
} saWriteSettings_t;

class SmartAudio {
public:
	SmartAudio(uint8_t rxtx);
	virtual ~SmartAudio();
	void setup();
	void getSettings();
	void writeChannel(saWriteSettings_t settings);
	void writePower(saWriteSettings_t settings);
	void writePitMode(saWriteSettings_t settings);
	bool processSmartAudio(uint32_t delayMs);
	uint8_t* getBuffer();

	void printBuffer(uint8_t *buffer, uint8_t size, const char* msg);

private:
	uint8_t _rxtx;
	SAMDHDUart _ss;
	uint8_t _buffer[2][SA_RX_BUFFER_LEN];
	uint8_t _rxBuffer;
	saState_e _saState;

	void sendFrame(uint8_t *sendBuffer, uint8_t size);
};

#endif /* SMARTAUDIO_H_ */
