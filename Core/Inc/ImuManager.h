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

namespace Osprai {

class ImuManager {
private:
	double accelRange;
	double gyroRange;
	double linAccelVect[3];
	double rotAccelVect[3];
	double orientation[3];
	bool computeChecksum= false;
	I2C_HandleTypeDef *sensorBus;
	void getAccelData(double *linAccelVect, bool enableInterrupt);
	void getGyroData(double *rotAccelVect, bool enableInterrupt);
public:
	ImuManager();
	virtual ~ImuManager();
	HAL_StatusTypeDef InitSensor(I2C_HandleTypeDef *bus);
	HAL_StatusTypeDef UpdatePose(bool enableInterrupt);
};

} /* namespace Osprai */

#endif /* SRC_IMUMANAGER_H_ */
