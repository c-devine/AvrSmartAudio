/*
 * Gps.h
 *
 *  Created on: Feb 1, 2018
 *      Author: Chris
 */

#ifndef GPS_H_
#define GPS_H_

#include <stdlib.h>
#include "Arduino.h"
#include "AvrSmartAudio.h"

#define GPS_BAUD 9600
#define GPS_BUFFER_LEN	128

class Gps {
public:
	Gps(Uart &serial, boolean echo = false);
	virtual ~Gps();
	void setup();
	void enable(boolean enabled);
	geodata_t* processSerial();
	void printGeodata(geodata_t *geoData);

private:

	void parseMessage();
	double parseLatLon(float latlon, char dir);
	void processGGA(char *buffer);
	void processRMC(char *buffer);
	void echo(const char *msg);

	Uart &_serial;
	char _buffer[GPS_BUFFER_LEN];
	char _message[GPS_BUFFER_LEN];
	boolean _echo = false;
	uint8_t _rxIndex = 0;
	geodata_t _geodata = {0,0,0,0,0,0,0};
	boolean _dataReady = false;
	boolean _enabled = false;
};

#endif /* GPS_H_ */
