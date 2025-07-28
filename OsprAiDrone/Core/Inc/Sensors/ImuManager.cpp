/*
 * ImuManager.cpp
 *
 *  Created on: Feb 3, 2025
 *      Author: nclsr
 */

#include <Sensors/ImuManager.h>
#include "main.h"
#include <cmath>

namespace Osprai {

ImuManager::ImuManager(int bufferSize) :SensorManager(bufferSize) {
	// TODO Auto-generated constructor stub

}

ImuManager::~ImuManager() {
	// TODO Auto-generated destructor stub
}

HAL_StatusTypeDef ImuManager::SensorConfiguration(map<uint8_t, vector<uint8_t>> *configuration) {
	this->timestep= HAL_GetTick()/(unsigned long)1000;
	I2C_HandleTypeDef *bus= getI2CInterface();
	uint8_t address1;
	uint8_t address2;
	uint8_t configVal;
	HAL_StatusTypeDef status= HAL_ERROR;
	//Verification connexion avec Capteur
	status= HAL_I2C_Mem_Read(bus, MPU1_SLAVE_ADDR, 0x75, 1, &address1, 1, 1000);
	status= HAL_I2C_IsDeviceReady(bus, MPU1_SLAVE_ADDR, 5, 1000);
	status= HAL_I2C_Mem_Read(bus, MPU2_SLAVE_ADDR, 0x75, 1, &address2, 1, 1000);
	status= HAL_I2C_IsDeviceReady(bus, MPU2_SLAVE_ADDR, 5, 1000);
	if (status != HAL_ERROR /*&& address >= 0x68 && address <= 0x71*/) {
		if (configuration == nullptr) {
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
			status = HAL_I2C_Mem_Write(bus, MPU1_SLAVE_ADDR, 0x1B, 1, &configVal, 1, 1000);
			status = HAL_I2C_Mem_Read(bus, MPU2_SLAVE_ADDR, 0x1B, 1, &configVal, 1, 1000);
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
			status = HAL_I2C_Mem_Write(bus, MPU1_SLAVE_ADDR, 0x1C, 1, &configVal, 1, 1000);
			status = HAL_I2C_Mem_Write(bus, MPU2_SLAVE_ADDR, 0x1C, 1, &configVal, 1, 1000);
			//Wake UP du capteur
				//Bit 7 -> 1: Hard_Reset of all Registers
				//Bit 6 -> 1: Sleep Mode
				//Bit 5 -> 1: Alternate SLEEP <-> StandBy Mode at LP_ACCEL_ODR register Freq
				//Bit 4 -> 1: Low Powr Mode, pulling for getting gyro sample data
				//Bit 3 -> 1: Disable Temperature Sensor
				//Bit 2/1/0 -> 111: Stop clock, timing gen in reset | 110 and 000: Internal 20MHz Clock
				//				001 -> 101: Auto select best clock freq
			configVal= 0b00000000;
			status = HAL_I2C_Mem_Write(bus, MPU1_SLAVE_ADDR, 0x6B, 1, &configVal, 1, 1000);
			status = HAL_I2C_Mem_Write(bus, MPU2_SLAVE_ADDR, 0x6B, 1, &configVal, 1, 1000);
		}
		//ExtractData(false);
	}
	return status;
}

HAL_StatusTypeDef ImuManager::getGyroData(double *rotAccelVect, bool enableInterrupt= false, int nSamples= 3) {
	I2C_HandleTypeDef *bus= getI2CInterface();
	HAL_StatusTypeDef status=HAL_ERROR;
	vector<double> xGyro;
	vector<double> yGyro;
	vector<double> zGyro;
	for (int j= 0; j < nSamples; j++) {
		uint8_t rawGyro1[6];
		int16_t signGyro1[3];
		uint8_t rawGyro2[6];
		int16_t signGyro2[3];
		if (enableInterrupt) {
			status= HAL_I2C_Mem_Read_IT(bus, MPU1_SLAVE_ADDR, 0x43, 1, rawGyro1, 6);
			status= HAL_I2C_Mem_Read_IT(bus, MPU2_SLAVE_ADDR, 0x43, 1, rawGyro2, 6);
		}
		else {
			status= HAL_I2C_Mem_Read(bus, MPU1_SLAVE_ADDR, 0x43, 1, rawGyro1, 6, 1000);
			status= HAL_I2C_Mem_Read(bus, MPU2_SLAVE_ADDR, 0x43, 1, rawGyro2, 6, 1000);
		}
		signGyro1[0] = (((int16_t)rawGyro1[0] << 8) | (rawGyro1[1] << 0));
		signGyro1[1] = (((int16_t)rawGyro1[2] << 8) | (rawGyro1[3] << 0));
		signGyro1[2] = (((int16_t)rawGyro1[4] << 8) + (rawGyro1[5] << 0));
		signGyro2[0] = (((int16_t)rawGyro2[0] << 8) | (rawGyro2[1] << 0));
		signGyro2[1] = (((int16_t)rawGyro2[2] << 8) | (rawGyro2[3] << 0));
		signGyro2[2] = (((int16_t)rawGyro2[4] << 8) + (rawGyro2[5] << 0));
		xGyro.push_back(signGyro1[0] / (pow((double)2, (double)16) / this->gyroRange));
		yGyro.push_back(signGyro1[1] / (pow((double)2, (double)16) / this->gyroRange));
		zGyro.push_back(signGyro1[2] / (pow((double)2, (double)16) / this->gyroRange));
		xGyro.push_back(signGyro2[0] / (pow((double)2, (double)16) / this->gyroRange));
		yGyro.push_back(signGyro2[1] / (pow((double)2, (double)16) / this->gyroRange));
		zGyro.push_back(signGyro2[2] / (pow((double)2, (double)16) / this->gyroRange));
	}
	rotAccelVect[0]= Median(xGyro);
	rotAccelVect[1]= Median(yGyro);
	rotAccelVect[2]= Median(zGyro);
	return status;
}

HAL_StatusTypeDef ImuManager::getAccelData(double *linAccelVect, bool enableInterrupt= false, int nSamples= 3) {
	I2C_HandleTypeDef *bus= getI2CInterface();
	HAL_StatusTypeDef status=HAL_ERROR;
	for (int i= 0; i < 3; i++){
		/*switch (i) {
			case 0:
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU0_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU2_Pin, GPIO_PIN_RESET);
				break;
			case 1:
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU1_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU2_Pin, GPIO_PIN_RESET);
				break;
			case 2:
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU0_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, ADDR_IMU2_Pin, GPIO_PIN_SET);
				break;
		}*/
		vector<double> xAccel;
		vector<double> yAccel;
		vector<double> zAccel;
		for (int j= 0; j < nSamples; j++) {
			uint8_t rawAccel[6];
			int16_t signAccel[3];
			if (enableInterrupt) {
				status= HAL_I2C_Mem_Read_IT(bus, MPU1_SLAVE_ADDR, 0x3B, 1, rawAccel, 6);
			}
			else {
				status= HAL_I2C_Mem_Read(bus, MPU1_SLAVE_ADDR, 0x3B, 1, rawAccel, 6, 1000);
			}
			signAccel[0] = (((int16_t)rawAccel[0] << 8) | (rawAccel[1] << 0));
			signAccel[1] = (((int16_t)rawAccel[2] << 8) | (rawAccel[3] << 0));
			signAccel[2] = (((int16_t)rawAccel[4] << 8) + (rawAccel[5] << 0));
			double a= signAccel[0] / (pow((double)2, (double)16) / this->accelRange);
			xAccel.push_back(signAccel[0] / (pow((double)2, (double)16) / this->accelRange));
			yAccel.push_back(signAccel[1] / (pow((double)2, (double)16) / this->accelRange));
			zAccel.push_back(signAccel[2] / (pow((double)2, (double)16) / this->accelRange));
		}
		double* t= xAccel.data();
		linAccelVect[0]= Median(xAccel);
		linAccelVect[1]= Median(yAccel);
		linAccelVect[2]= Median(zAccel);
	}
	return status;
}

HAL_StatusTypeDef ImuManager::ExtractData(bool enableInterrupt= false) {
	//X data accel
	HAL_StatusTypeDef status= getAccelData(this->linAccelVect, enableInterrupt);
	status= getGyroData(this->rotAccelVect, enableInterrupt);
	double currentTimestep= this->timestep;
	if (status == HAL_OK) {		//X Y Z X Y
		currentTimestep= (double)(HAL_GetTick()) / 1000.0;
		this->orientation[0] += this->rotAccelVect[0]*(currentTimestep - this->timestep);//atan2(this->linAccelVect[1], this->linAccelVect[2]);
		this->orientation[1] += this->rotAccelVect[1]*(currentTimestep - this->timestep); //atan2(this->linAccelVect[2], this->linAccelVect[0]);
		this->orientation[2] += this->rotAccelVect[2]*(currentTimestep - this->timestep);//atan2(this->linAccelVect[1], this->linAccelVect[0]);
		this->timestep = currentTimestep;
		double accNorm= sqrt(pow(this->linAccelVect[0], 2) + pow(this->linAccelVect[1], 2) + pow(this->linAccelVect[2], 2));
		if (accNorm > 0.0) {
			this->linAccelVect[0] = (this->linAccelVect[0]/accNorm)*(accNorm-1);
			this->linAccelVect[1] = (this->linAccelVect[1]/accNorm)*(accNorm-1);
			this->linAccelVect[2] = (this->linAccelVect[2]/accNorm)*(accNorm-1);
		}
	}
	return status;
}

HAL_StatusTypeDef ImuManager::AnswerToRequest(vector<uint8_t> request) {
	return HAL_OK;
}

} /* namespace Osprai */
