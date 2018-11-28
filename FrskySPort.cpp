#include "FrskySPort.h"
#include "variant.h"

FrskySPort::FrskySPort(uint8_t rxtx) :
		_ss(rxtx, rxtx, SPORT_TIMER, SPORT_BAUD, true) {

	_rxtx = rxtx;
	_crc = 0;
	_readBuffer = 0;
	_spState = SP_IDLE;
	memset(_buffer[0], 0, SP_RX_BUFFER_LEN);
	memset(_buffer[1], 0, SP_RX_BUFFER_LEN);
}

FrskySPort::~FrskySPort() {

	_ss.end();
}

void FrskySPort::setup() {

	_ss.begin();
}

uint8_t FrskySPort::waitNext() {

	while (!_ss.available()) {
	}
	return _ss.read();
}

mspFrame_t* FrskySPort::checkMSP() {

	static uint8_t bufferPos = 0;
	static uint8_t numPkts = 0;

	if (!_ss.isListening()) {
		_ss.listen();
		//Serial.println("listening SPort");
	}

	while (_ss.available()) {

		uint8_t byte = _ss.read();
		switch (_spState) {

		default:
		case SP_IDLE:

			if (byte == START_STOP) {
				_spState = SP_START_STOP;
			}
			break;

		case SP_START_STOP:

			if (byte == SENSOR_ID) {
				_spState = SP_SENSOR_ID;
			} else
				_spState = SP_IDLE;
			break;

		case SP_SENSOR_ID:

			if (byte == REQUEST_FRAME_ID) {
				_spState = SP_DATA_READ;
			} else {
				_spState = SP_IDLE;
			}
			break;

		case SP_DATA_READ:

			_buffer[_readBuffer][bufferPos++] = byte;

			if (bufferPos == 2)
				numPkts = ((byte + 3) / 6) + 1; // need to add seq, size, and command to payload size

			if (bufferPos % 6 == 0) {
				_spState = SP_IDLE;
				if (bufferPos == numPkts * 6) {

					_readBuffer = !_readBuffer;
					memset(_buffer[_readBuffer], 0, SP_RX_BUFFER_LEN);
					bufferPos = 0;

					return getBuffer();
				}

			}
			break;
		}
	}

	return NULL;
}

mspFrame_t* FrskySPort::getBuffer() {

	return (mspFrame_t*) _buffer[!_readBuffer];
}

void FrskySPort::sendData(uint8_t sensorId, uint8_t frameId, uint16_t dataId,
		uint32_t payload) {

	uint8_t data = 0;
	uint8_t lastRx = 0;

	if (!_ss.isListening()) {
		_ss.listen();
		//Serial.println("listening SPort");
	}

	while (true) {
		if (_ss.available()) {
			data = _ss.read();
			if (data == sensorId && lastRx == START_STOP) {
				_ss.stopListening();
				_ss.setTX();
				sendPayload(frameId, dataId, payload);
				break;
			}
			lastRx = data;
		}
	}
}

void FrskySPort::sendByte(uint8_t byte) {

	//Serial.print(byte, HEX);
	//Serial.print(" ");
	_ss.write(byte);
	// CRC update
	_crc += byte; //0-1FF
	_crc += _crc >> 8; //0-100
	_crc &= 0x00ff;
	_crc += _crc >> 8; //0-0FF
	_crc &= 0x00ff;
}


void FrskySPort::sendCrc() {

	//Serial.println(0xFF - _crc, HEX);
	_ss.write(0xFF - _crc);
	// CRC reset
	_crc = 0;
}

void FrskySPort::sendPayload(uint8_t frameId, uint16_t dataId,
		int32_t payload) {

	sendByte(frameId);
	uint8_t *bytes = (uint8_t*) &dataId;
	sendByte(bytes[0]);
	sendByte(bytes[1]);

	bytes = (uint8_t*) &payload;
	sendByte(bytes[0]);
	sendByte(bytes[1]);
	sendByte(bytes[2]);
	sendByte(bytes[3]);

	sendCrc();
}

void FrskySPort::sendLatLon(geodata_t* geoData) {

	int32_t gpsLat = (geoData->latitude * 60.0) * 10000;

	if (gpsLat < 0) {
		gpsLat *= -1.0f;
		gpsLat |= (FrskySPort::S << 30);
	}

	int32_t gpsLon = (uint32_t) ((geoData->longitude * 60.0) * 10000);

	if (gpsLon < 0) {
		gpsLon *= -1.0f;
		gpsLon |= (FrskySPort::W << 30);
	} else
		gpsLon |= (FrskySPort::E << 30);

	sendData(SENSOR_ID_GPS, FRSKY_FRAME_ID, DATA_ID_GPS_LONLAT, gpsLon);
	sendData(SENSOR_ID_GPS, FRSKY_FRAME_ID, DATA_ID_GPS_LONLAT, gpsLat);
}
