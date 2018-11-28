#include "Gps.h"

Gps::Gps(Uart &serial, boolean echo) :
		_serial(serial) {

	_echo = echo;
}

Gps::~Gps() {

}

void Gps::setup() {

	_serial.begin(GPS_BAUD);
	enable(true);
}

void Gps::enable(boolean enabled) {

	_enabled = enabled;
}

void Gps::echo(const char *msg) {

	if (_echo)
		Serial.println(msg);
}

void Gps::printGeodata(geodata_t *geoData) {

	Serial.print("lat = ");
	Serial.println(geoData->latitude);
	Serial.print("lon = ");
	Serial.println(geoData->longitude);
	Serial.print("fix = ");
	Serial.println(geoData->gpsFix);
	Serial.print("num sats = ");
	Serial.println(geoData->numSats);
}

void Gps::parseMessage() {

	echo(_message);

	if (strncmp(_message, "$GPRMC", 6) == 0) {
		processRMC(_message);
	} else if (strncmp(_message, "$GPGGA", 6) == 0) {
		processGGA(_message);
	}
}

double Gps::parseLatLon(float latlon, char dir) {

	int degrees = (int) (latlon / 100);
	float minutes = latlon - (degrees * 100);
	float value = degrees + (minutes / 60.0);
	if (dir == 'S' || dir == 'W')
		return value * -1.0f;
	else
		return value;
}

void Gps::processGGA(char *buffer) {

	//$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
	strtok(buffer, ",");
	strtok(NULL, ",");
	_geodata.latitude = parseLatLon(atof(strtok(NULL, ",")),
			*strtok(NULL, ","));
	_geodata.longitude = parseLatLon(atof(strtok(NULL, ",")),
			*strtok(NULL, ","));
	char *tmp = strtok(NULL, ",");
	_geodata.gpsFix = strlen(tmp) ? atoi(tmp) : 0;
	tmp = strtok(NULL, ",");
	_geodata.numSats = strlen(tmp) ? atoi(tmp) : 0;
	strtok(NULL, ",");
	_geodata.altitude = atof(strtok(NULL, ","));
}

void Gps::processRMC(char *buffer) {

	// $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
	strtok(buffer, ",");
	strtok(NULL, ",");
	strtok(NULL, ",");
	_geodata.latitude = parseLatLon(atof(strtok(NULL, ",")),
			*strtok(NULL, ","));
	_geodata.longitude = parseLatLon(atof(strtok(NULL, ",")),
			*strtok(NULL, ","));
}

geodata_t* Gps::processSerial() {

	if (!_enabled)
		return NULL;

	while (_serial.available()) {

		char c = _serial.read();

		if (c == '\r')
			continue;

		// check for start of message
		if (_rxIndex == 0 && c != '$')
			continue;

		// look for end of NMEA message
		if (c == '\n') {
			_rxIndex = 0;
			strncpy(_message, _buffer, GPS_BUFFER_LEN);
			memset(_buffer, 0, GPS_BUFFER_LEN);

			if ((strncmp(_message, "$GPRMC", 6) == 0)
					|| (strncmp(_message, "$GPGGA", 6) == 0)) {
				parseMessage();
				return &_geodata;
			} else
				return NULL;
		}

		// put the byte into the buffer
		_buffer[_rxIndex++] = c;
		if (_rxIndex == GPS_BUFFER_LEN)
			_rxIndex = 0;
	}

	return NULL;
}

