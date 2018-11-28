#include "Vtx.h"
#include "VtxLookup.h"

void printMspFrame(mspFrame_t* frame, const char* msg) {

	Serial.print(msg);
	Serial.print(" seq: ");
	Serial.print(frame->seq, HEX);
	Serial.print(" id: ");
	Serial.print(frame->seq & 0xF);
	Serial.print(" start: ");
	Serial.print(MSP_START_FRAME(frame->seq) ? 1 : 0);
	Serial.print(" size: ");
	Serial.print(frame->size, HEX);
	for (uint8_t i = 0; i < 6; i++) {
		Serial.print(" data[");
		Serial.print(i);
		Serial.print("] = ");
		Serial.print(frame->data[i], HEX);
	}
	Serial.println();

}

Vtx::Vtx(FrskySPort &frsport, SmartAudio &sa) :
		_frsport(frsport), _sa(sa) {

	_vtxState = VTX_IDLE;
	_lastCommand = 0;
	_lastMSPSeq = 0;

}

Vtx::~Vtx() {

}

void Vtx::setup() {

	_sa.setup();
	_frsport.setup();
}

void Vtx::loop() {
	static uint32_t mc = millis();

	switch (_vtxState) {

	case VTX_IDLE: {

		mspFrame_t* mspFrame = filterMspFrame();
		if (mspFrame == NULL)
			return;

		// add the frame to the queue
		mspFrame_t* newFrame = new mspFrame_t;
		memcpy(newFrame, mspFrame, sizeof(mspFrame_t));
		_mspFrames.push(newFrame);
		handleMSP();
		return;
	}

	case VTX_SA_WAIT:

		if (_sa.processSmartAudio(200)) {
			handleSAResponse();
			_vtxState = VTX_IDLE;
			return;
		}

		if (millis() - mc > 2000) {
			_vtxState = VTX_IDLE;
			mc = millis();
		}
		return;
	}

}

/**
 * Returns only read and write msp frames, as well as tries to figure out if pit mode is set
 */

mspFrame_t* Vtx::filterMspFrame() {

	mspFrame_t* mspFrame = _frsport.checkMSP();

	// return null or not a read or write frame
	if (mspFrame == NULL
			|| !(getCommand(mspFrame) == MSP_VTX_READ
					|| getCommand(mspFrame) == MSP_VTX_WRITE))
		return NULL;

	//printMspFrame(mspFrame, "filter frame");

	return mspFrame;

}

void Vtx::handleSAResponse() {

	switch (_lastCommand) {

	case MSP_VTX_READ: {

		// 0xAA 0x55 0x01 (Version/Command) 0x06 (Length) 0x00 (Channel) 0x00 (Power Level) 0x01(Operation
		// Mode) 0x16 0xE9(Current Frequency 5865) 0x4D(CRC8)
		uint8_t *buffer = _sa.getBuffer();
		//_sa.printBuffer(buffer, buffer[1] + 2, "handleSAResponse MSP_VTX_READ");

		_saDevice.version = (buffer[0] == 0x01) ? 1 : 2;
		_saDevice.channel = buffer[2];
		_saDevice.power = buffer[3] + 1;
		_saDevice.mode = buffer[4];
		_saDevice.freq_h = buffer[5]; // not sure this is implemented in SA device
		_saDevice.freq_l = buffer[6]; // not sure this is implemented in SA device

		uint8_t band = SA_DEVICE_CHVAL_TO_BAND(_saDevice.channel) + 1;
		uint8_t channel = SA_DEVICE_CHVAL_TO_CHANNEL(_saDevice.channel) + 1;
		uint16_t freq = vtx58_Bandchan2Freq(band, channel);

		uint8_t crc = 0;
		uint8_t messageSize = 7;
		uint32_t payload = 0;
		uint8_t *bytes = (uint8_t*) &payload;
		bytes[0] = MSP_SA_PROTOCOL;
		bytes[1] = band; // band
		bytes[2] = channel; //_saDevice.channel;
		bytes[3] = _saDevice.power;

		uint16_t header = (_lastMSPSeq & 0xF) | MSP_START_FLAG
				| (messageSize << 8);

		crc = MSP_VTX_READ ^ messageSize ^ bytes[0] ^ bytes[1] ^ bytes[2]
				^ bytes[3];

		_frsport.sendData(MSP_RESPONSE_SENSOR_ID, RESPONSE_FRAME_ID, header, payload);

		uint8_t pitMode =
				_saDevice.mode
						& (SA_OPMODE_PITMODE_ACTIVE | SA_OPMODE_IN_RANGE_PITMODE) ?
						1 : 0;
		header = ((++_lastMSPSeq) & 0xF) | (pitMode << 8);

		bytes[0] = freq & 0xFF; // freq & 0xFF; // low byte
		bytes[1] = freq >> 8; //freq >> 8; // high byte
		bytes[2] = crc ^ pitMode ^ bytes[0] ^ bytes[1];
		bytes[3] = 0;

		_frsport.sendData(MSP_RESPONSE_SENSOR_ID, RESPONSE_FRAME_ID, header, payload);
		return;
	}
	case MSP_VTX_WRITE:

		uint8_t *buffer = _sa.getBuffer();
		//_sa.printBuffer(buffer, buffer[1] + 2,
		//		"handleSAResponse MSP_VTX_WRITE");

		uint16_t header = (_lastMSPSeq & 0xF) | MSP_START_FLAG | (0 << 8);
		//uint8_t crc = MSP_VTX_WRITE;
		uint32_t payload = 0;
		uint8_t *bytes = (uint8_t*) &payload;
		bytes[0] = MSP_VTX_WRITE;

		_frsport.sendData(MSP_RESPONSE_SENSOR_ID, RESPONSE_FRAME_ID, header,
				payload);

		header = (++_lastMSPSeq & 0xF) | MSP_START_FLAG | (0 << 8);
		bytes[0] = MSP_EEPROM_WRITE;

		_frsport.sendData(MSP_RESPONSE_SENSOR_ID, RESPONSE_FRAME_ID, header,
				payload);

		return;

	}

}

void Vtx::handleMSP() {

	if (_mspFrames.isEmpty())
		return;

	mspFrame_t* mspFrame = _mspFrames.shift();

	if (getCommand(mspFrame) == MSP_VTX_READ) {
		_lastCommand = MSP_VTX_READ;
		_lastMSPSeq = mspFrame->seq;
		// get the smart audio settings
		_sa.getSettings();
		_vtxState = VTX_SA_WAIT;
	}

	else if (getCommand(mspFrame) == MSP_VTX_WRITE) {
		_lastCommand = MSP_VTX_WRITE;
		_lastMSPSeq = mspFrame->seq;
		_saWriteSettings.channel = mspFrame->data[1];
		_saWriteSettings.pitMode = mspFrame->data[5];
		_saWriteSettings.power = mspFrame->data[3] - 1;

		_sa.writeChannel(_saWriteSettings);
		_sa.processSmartAudio(200);
		_sa.writePower(_saWriteSettings);
		_sa.processSmartAudio(200);
		_sa.writePitMode(_saWriteSettings);
		//_sa.processSmartAudio(200);
		_vtxState = VTX_SA_WAIT;

	}

	delete mspFrame;

	return;
}

uint8_t Vtx::getCommand(mspFrame_t* mspFrame) {

	return mspFrame->data[0];
}
