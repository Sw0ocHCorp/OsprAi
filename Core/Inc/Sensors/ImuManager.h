/*
 * ImuManager.h
 *
 *  Created on: Feb 3, 2025
 *      Author: nclsr
 */

#ifndef SRC_IMUMANAGER_H_
#define SRC_IMUMANAGER_H_

#define MPU_SLAVE_ADDR 0xD0

#include "main.h"
#include "SensorManager.h"

namespace Osprai {

class ImuManager: public SensorManager {
private:
	double accelRange;
	double gyroRange;
	double linAccelVect[3];
	double rotAccelVect[3];
	double orientation[3];
	bool computeChecksum= false;
	HAL_StatusTypeDef getAccelData(double *linAccelVect, bool enableInterrupt);
	HAL_StatusTypeDef getGyroData(double *rotAccelVect, bool enableInterrupt);
	HAL_StatusTypeDef ExtractData(bool enableInterrupt);
	HAL_StatusTypeDef AnswerToRequest(vector<uint8_t> request);
public:
	ImuManager(int bufferSize);
	virtual ~ImuManager();
	HAL_StatusTypeDef SensorConfiguration(map<uint8_t, vector<uint8_t>> *configuration);
};

} /* namespace Osprai */

#endif /* SRC_IMUMANAGER_H_ */
