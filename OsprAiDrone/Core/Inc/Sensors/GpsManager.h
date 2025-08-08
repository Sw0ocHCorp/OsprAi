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

#define TIMESTEP 		0
#define POSE_VALIDATION	1
#define LATITUDE		2
#define LONGITUDE   	3
#define SPEED			4
#define COURSE			5
#define DATETIME		6
#define MAGNETIC_VAR	7
#define CHECKSUM		8
#define INVALID			9

#include "UARTSensor.h"

namespace OsprAi {

class GpsManager : public UARTSensor {
private:
	char Separator;
	uint8_t IncomingData[250];

public:

	GpsManager() {

	}

	virtual ~GpsManager() {

	}



	HAL_StatusTypeDef TakeMeasurement() {
		HAL_UART_Receive_IT(UartInterface, IncomingData, 250);
	}

	void ProcessMeasurement() {
		vector<uint8_t> receivedData(IncomingData, IncomingData + 250);
		int  startIndex= FindPattern(receivedData, vector<uint8_t> {'G', 'P', 'R', 'M', 'C'});
		//We process Frames in receivedData
		if(startIndex >=0) {
			vector<uint8_t> frameData(receivedData.begin() + startIndex, receivedData.end());
			int endIndex= FindPattern(frameData, vector<uint8_t> {'*'});
			if (endIndex >= 0) {
				vector<uint8_t> frame(frameData.begin(), frameData.begin() + endIndex+3);
				//ParseFrame(frame);
			}

		}
	}

	void SetSeparator(char separator) { Separator = separator; }
};

} /* namespace Osprai */

#endif /* INC_GPSMANAGER_H_ */
