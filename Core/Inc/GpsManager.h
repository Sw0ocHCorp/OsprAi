/*
 * GpsManager.h
 *
 *  Created on: Jan 18, 2025
 *      Author: nclsr
 */

#ifndef INC_GPSMANAGER_H_
#define INC_GPSMANAGER_H_

#include "main.h"
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

namespace Osprai {

class GpsManager {
private:
	//Sensor Infos
	UART_HandleTypeDef *sensorBus;
	tm currentTime;
	double longitude;
	double latitude;
	double speed;
	double course;
	double magneticVar;
	//Var for Frame Parsing
	bool computeChecksum= false;
	uint8_t sof;
	uint8_t separator;
	int bufferIndex= -1;
	uint8_t checksum;
	HAL_StatusTypeDef ParseFrame();
	double ParseLatLong(string *buff);
	double ParseSpeedAngle(string *buff);
	double ParseMagneticVar(string *buff);
	uint8_t ComputeCheckSum();
public:
	uint8_t incomingByte= 0;
	uint8_t *buffer;
	GpsManager();
	virtual ~GpsManager();
	HAL_StatusTypeDef InitGps(UART_HandleTypeDef *bus, uint8_t sof, uint8_t separator);
	void UpdateLocation();
	UART_HandleTypeDef *GetBus();
	double GetLongitude();
	double GetLatitude();
};

} /* namespace Osprai */

#endif /* INC_GPSMANAGER_H_ */
