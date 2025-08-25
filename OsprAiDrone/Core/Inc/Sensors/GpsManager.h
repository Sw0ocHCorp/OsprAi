/*
 * GpsManager.h
 *
 *  Created on: Jan 18, 2025
 *      Author: nclsr
 */

#ifndef INC_GPSMANAGER_H_
#define INC_GPSMANAGER_H_

#include "main.h"
#include "utils.h"
#include <string>
#include <ctime>

using namespace std;

#define KNOT_MS		0.51444444
#define DATA_LEN	256
#define POSE_ID "GPRMC"

#define TIMESTAMP 		0
#define POSE_VALIDATION	1
#define LATITUDE		2
#define LONGITUDE   	3
#define SPEED			4
#define HEADING			5
#define DATETIME		6
#define MAGNETIC_VAR	7
#define CHECKSUM		8
#define INVALID			9

#include "UARTSensor.h"

namespace OsprAi {

/*class GpsManager : public UARTSensor {
private:
	float Latitude;
	float Longitude;
	float Speed;
	float Heading;
	bool IsValidData;

	float ParseLatLon(uint8_t *buffer, bool isLat) {
		char degVec[3];
		float deg= 0;
		float min = 0;
		if (isLat) {
			strncpy(degVec, (char *)buffer, 2);
			vector<char> minVec(buffer+2, buffer+10);
			deg = atof(degVec);
			min = atof(minVec.data());
		} else {
			float lon;
			strncpy(degVec, (char *)buffer, 3);
			vector<char> minVec(buffer+3, buffer+10);
			deg = atof(degVec);
			min = atof(minVec.data());
		}
		return deg + min / 60.0;
		//FindPattern(targetStr, pattern, findFromEnd)
	}

public:
	uint8_t IncomingBytes[150];
	uint8_t State = TIMESTAMP;
	GpsManager() {
	}

	virtual ~GpsManager() {

	}

	void TakeMeasurement() {

		int startIndex= FindPattern(IncomingBytes, 150, new uint8_t[5] {'G', 'P', 'R', 'M', 'C'}, 5);
		if (startIndex < 0) {
			HAL_UART_Receive_IT(UartInterface, IncomingBytes, 150);
			IsValidData = false;
		}
		else {
			vector<uint8_t> frameVec(IncomingBytes + startIndex, IncomingBytes + 150);
			int endIndex = FindPattern(frameVec, {'*'}, true);
			if (endIndex < 0) {
				HAL_UART_Receive_IT(UartInterface, IncomingBytes, 150);
				IsValidData = false;
			}
			else {
				IsValidData = true;
				State= 0;
				uint8_t checksum= 0;
				uint8_t buffer[16];
				int bufferIndex= 0;

				memset(buffer, 0, 16);
				for(int i= startIndex; i < 150; i++) {
					if (IsValidData == false)
						break;
					if (IncomingBytes[i] != '*') {
						checksum ^= IncomingBytes[i];
					}
					if (i > startIndex + 5) {
						if (IncomingBytes[i] == ',') {
							if (State == TIMESTAMP) {
							} else if (State == POSE_VALIDATION &&
											buffer[0] != 'A') {
								IsValidData = false;
							} else if (State == LATITUDE) {
								Latitude= ParseLatLon(buffer, true);
							} else if (State == LONGITUDE) {
								Longitude= ParseLatLon(buffer, false);
							} else if (State == SPEED) {
								Speed= atof((char *)buffer) * KNOT_MS;
							} else if (State == HEADING) {
								Heading= atof((char *)buffer);
								Heading= Deg2Rad(Heading, true);
							}
							if (State == LATITUDE || State == LONGITUDE) {
								checksum ^= (IncomingBytes[i+1] ^ IncomingBytes[i+2]);
								i += 2;
							}
							State++;
							bufferIndex= 0;
							memset(buffer, 0, 16);
						} else if (IncomingBytes[i] == '*') {
							uint8_t realChecksum= CharToByte(IncomingBytes[i+1]) << 4 | CharToByte(IncomingBytes[i+2]) << 0;
							if (realChecksum == checksum) {
								IsValidData = true;
							}
							else {
								IsValidData = false;
							}
							break;
						}else {
							buffer[bufferIndex] = IncomingBytes[i];
							bufferIndex++;
						}
					}
				}
				memset(IncomingBytes, 0, 150);
			}
		}

	}
};*/

} /* namespace Osprai */

#endif /* INC_GPSMANAGER_H_ */
