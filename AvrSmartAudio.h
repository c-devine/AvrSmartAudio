#ifndef AVRSMARTAUDIO_H_
#define AVRSMARTAUDIO_H_

typedef struct {
	float latitude;
	float longitude;
	float altitude;
	uint8_t numSats;
	uint8_t gpsFix;
	float hdop;
	float geoid;

} geodata_t;


#endif /* AVRSMARTAUDIO_H_ */
