/*
 * GpsManager.h
 *
 *  Created on: Jan 18, 2025
 *      Author: nclsr
 */

#ifndef INC_GPSMANAGER_H_
#define INC_GPSMANAGER_H_

#include "main.h"
#include "SensorManager.h"
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

class GpsManager : public SensorManager {
private:
	//Sensor Infos
	tm currentTime;
	double longitude;
	double latitude;
	double speed;
	double course;
	double magneticVar;
	//Var for Frame Parsing
	bool computeChecksum= false;
	uint8_t checksum;
	int bufferIndex= -1;
	uint8_t sof= '$';
	HAL_StatusTypeDef ParseFrame();
	double ParseLatLong(string *buff);
	double ParseSpeedAngle(string *buff);
	double ParseMagneticVar(string *buff);
	uint8_t ComputeNMEACheckSum(vector<uint8_t> frame);
	HAL_StatusTypeDef AnswerToRequest(vector<uint8_t> request);
	HAL_StatusTypeDef ExtractData(bool enableInterrupt);
public:
	uint8_t incomingByte= 0;
	uint8_t *buffer;
	vector<uint8_t> frameBuffer;
	GpsManager(int bufferSize);
	virtual ~GpsManager();
	HAL_StatusTypeDef UpdateLocation();
	HAL_StatusTypeDef SensorConfiguration(map<uint8_t, vector<uint8_t>> *configuration= nullptr);
};

} /* namespace Osprai */

#endif /* INC_GPSMANAGER_H_ */
