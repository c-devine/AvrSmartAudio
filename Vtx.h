#ifndef VTX_H_
#define VTX_H_

#include "Arduino.h"
#include "FrskySPort.h"
#include "SmartAudio.h"
#include "CircularBuffer.h"

#define MSP_ERROR_FLAG 0x20
#define MSP_START_FLAG 0x10
#define MSP_RESPONSE_SENSOR_ID 0

#define MSP_VTX_READ 0x58
#define MSP_VTX_WRITE 0x59
#define MSP_EEPROM_WRITE 0xFA

#define MSP_START_FRAME(val) ((val) & MSP_START_FLAG)


typedef enum {
	VTX_IDLE, VTX_SA_WAIT
} vtxState_e;

typedef struct smartAudioDevice_s {
	uint8_t version;
	uint8_t channel;
	uint8_t power;
	uint8_t mode;
	uint8_t freq_h;
	uint8_t freq_l;
} smartAudioDevice_t;

class Vtx {
public:
	Vtx(FrskySPort &frsport, SmartAudio &sa);
	virtual ~Vtx();
	void setup();
	void loop();

private:

	mspFrame_t* filterMspFrame();
	void handleMSP();
	void handleSAResponse();
	uint8_t getCommand(mspFrame_t* mspFrame);

	FrskySPort &_frsport;
	SmartAudio &_sa;
	CircularBuffer<mspFrame_t*, 4> _mspFrames;
	smartAudioDevice_t _saDevice;
	saWriteSettings_t _saWriteSettings;
	vtxState_e _vtxState;
	uint8_t _lastCommand;
	uint8_t _lastMSPSeq;
};

#endif /* VTX_H_ */
