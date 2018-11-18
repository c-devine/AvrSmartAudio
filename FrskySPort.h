#ifndef FRSPORT_H
#define FRSPORT_H

#include "Arduino.h"
#include "SAMDHDUart.h"

#define SPORT_TIMER 3

#define START_STOP 0x7E
#define SENSOR_ID 0x0D
#define REQUEST_FRAME_ID 0x30
#define RESPONSE_FRAME_ID 0x32

#define SPORT_BAUD 57600
#define SP_RX_BUFFER_LEN 32

typedef struct mspFrame_s {
	uint8_t seq;  // 0x10 = start, 0x0F = id
	uint8_t size;
	uint8_t data[30];

} mspFrame_t;

typedef enum {
	SP_IDLE, SP_START_STOP, SP_SENSOR_ID, SP_DATA_READ
} spState_e;

class FrskySPort {
public:
	FrskySPort(uint8_t rxtx);
	~FrskySPort();
	void setup();
	mspFrame_t* checkMSP();
	void sendData(uint8_t sensorId, uint16_t dataId, uint32_t payload);


private:
	uint8_t _rxtx;
	int16_t _crc;
	SAMDHDUart _ss;
	uint8_t _buffer[2][SP_RX_BUFFER_LEN];
	uint8_t _readBuffer;
	spState_e _spState;

	uint8_t waitNext();

	mspFrame_t* getBuffer();
	void sendPayload(uint16_t dataId, int32_t payload);
	void sendByte(uint8_t byte);
	void sendCrc();
};

#endif /*FRSPORT_H*/
