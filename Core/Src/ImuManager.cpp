/*
 * ImuManager.cpp
 *
 *  Created on: Feb 3, 2025
 *      Author: nclsr
 */

#include <ImuManager.h>
#include "main.h"
#include <cmath>

namespace Osprai {

ImuManager::ImuManager() {
	// TODO Auto-generated constructor stub

}

ImuManager::~ImuManager() {
	// TODO Auto-generated destructor stub
}

HAL_StatusTypeDef ImuManager::InitSensor(I2C_HandleTypeDef *bus) {
	this->sensorBus = bus;
	uint8_t address;
	uint8_t configVal;
	//Verification connexion avec Capteur
	HAL_StatusTypeDef status= HAL_I2C_Mem_Read(sensorBus, MPU_SLAVE_ADDR, 0x75, 1, &address, 1, 1000);
	status= HAL_I2C_IsDeviceReady(bus, MPU_SLAVE_ADDR, 5, 1000);
	if (status != HAL_ERROR) {
		//Gyro Config
			//Bit 7 -> 1: Perform X Gyro Self-Test
			//Bit 6 -> 1: Perform Y Gyro Self-Test
			//Bit 5 -> 1: Perform Z Gyro Self-Test
			//Bit 4/3 -> 00: +250dps | 01: +500dps | 10: +1000dps | 11: +2000dps	Range measurement
			//Bit 2 -> 0: Reserved
			//Bit 1/0 -> 00: Fchoice
		configVal= 0b00011000;
		if (configVal == 0b00000000)
			this->gyroRange = 250;
		else if (configVal == 0b00001000)
			this->gyroRange = 500;
		else if(configVal == 0b00010000)
			this->gyroRange = 1000;
		else if (configVal == 0b00011000)
			this->gyroRange= 2000;
		status = HAL_I2C_Mem_Write(sensorBus, MPU_SLAVE_ADDR, 0x1B, 1, &configVal, 1, 1000);
		//Accelero Config
			//Bit 7 -> 1: Perform X Accel Self-Test
			//Bit 6 -> 1: Perform Y Accel Self-Test
			//Bit 5 -> 1: Perform Z Accel Self-Test
			//Bit 4/3 -> 00: +/-2G | 01: +/-4G | 10: +/-8G | 11: +/-16G	Range measurement
			//Bit 2 -> 0: Reserved
			//Bit 1/0 -> 00: Fchoice
		configVal= 0b00011000;
		if (configVal == 0b00000000)
			this->accelRange = 4;
		else if (configVal == 0b00001000)
			this->accelRange = 8;
		else if(configVal == 0b00010000)
			this->accelRange = 16;
		else if (configVal == 0b00011000)
			this->accelRange= 32;
		status = HAL_I2C_Mem_Write(sensorBus, MPU_SLAVE_ADDR, 0x1C, 1, &configVal, 1, 1000);
		//Wake UP du capteur
			//Bit 7 -> 1: Hard_Reset of all Registers
			//Bit 6 -> 1: Sleep Mode
			//Bit 5 -> 1: Alternate SLEEP <-> StandBy Mode at LP_ACCEL_ODR register Freq
			//Bit 4 -> 1: Low Powr Mode, pulling for getting gyro sample data
			//Bit 3 -> 1: Disable Temperature Sensor
			//Bit 2/1/0 -> 111: Stop clock, timing gen in reset | 110 and 000: Internal 20MHz Clock
			//				001 -> 101: Auto select best clock freq
		configVal= 0b00000000;
		status = HAL_I2C_Mem_Write(sensorBus, MPU_SLAVE_ADDR, 0x6B, 1, &configVal, 1, 1000);

	}
	UpdatePose(false);
	return status;
}

void ImuManager::getGyroData(double *rotAccelVect, bool enableInterrupt= false) {
	uint8_t rawGyro[6];
	HAL_StatusTypeDef status=HAL_ERROR;
	int16_t signGyro [3];
	if (enableInterrupt) {
		status= HAL_I2C_Mem_Read_IT(sensorBus, MPU_SLAVE_ADDR, 0x43, 1, rawGyro, 6);
	}
	else {
		status= HAL_I2C_Mem_Read(sensorBus, MPU_SLAVE_ADDR, 0x43, 1, rawGyro, 6, 1000);
	}
	signGyro[0] = (((int16_t)rawGyro[0] << 8) | (rawGyro[1] << 0));
	signGyro[1] = (((int16_t)rawGyro[2] << 8) | (rawGyro[3] << 0));
	signGyro[2] = (((int16_t)rawGyro[4] << 8) + (rawGyro[5] << 0));
	for(int i= 0; i < 3; i++) {
		rotAccelVect[i] = signGyro[i] / (pow((double)2, (double)16) / this->gyroRange);
	}
}

void ImuManager::getAccelData(double *linAccelVect, bool enableInterrupt= false) {
	uint8_t rawAccel[6];
	int16_t signAccel[3];
	HAL_StatusTypeDef status=HAL_ERROR;
	if (enableInterrupt) {
		status= HAL_I2C_Mem_Read_IT(sensorBus, MPU_SLAVE_ADDR, 0x3B, 1, rawAccel, 6);
	}
	else {
		status= HAL_I2C_Mem_Read(sensorBus, MPU_SLAVE_ADDR, 0x3B, 1, rawAccel, 6, 1000);
	}
	signAccel[0] = (((int16_t)rawAccel[0] << 8) | (rawAccel[1] << 0));
	signAccel[1] = (((int16_t)rawAccel[2] << 8) | (rawAccel[3] << 0));
	signAccel[2] = (((int16_t)rawAccel[4] << 8) + (rawAccel[5] << 0));
	for(int i= 0; i < 3; i++) {
		linAccelVect[i] = signAccel[i] / (pow((double)2, (double)16) / this->accelRange);
	}
}

HAL_StatusTypeDef ImuManager::UpdatePose(bool enableInterrupt= false) {
	//X data accel
	getAccelData(this->linAccelVect, enableInterrupt);
	//X Y Z X Y
	this->orientation[2] = atan2(this->linAccelVect[1], this->linAccelVect[0]);
	this->orientation[1] = atan2(this->linAccelVect[2], this->linAccelVect[0]);
	this->orientation[0] = atan2(this->linAccelVect[1], this->linAccelVect[2]);
	getGyroData(this->rotAccelVect, enableInterrupt);
	int z= 1;
}

} /* namespace Osprai */
