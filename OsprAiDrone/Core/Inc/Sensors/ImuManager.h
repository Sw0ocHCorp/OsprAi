/*
 * ImuManager.h
 *
 *  Created on: Feb 3, 2025
 *      Author: nclsr
 */

#ifndef SRC_IMUMANAGER_H_
#define SRC_IMUMANAGER_H_

#define MPU1_SLAVE_ADDR 0x68 << 1
#define MPU2_SLAVE_ADDR 0x69 << 1
#define MPU_ADDR_ID		0xff
#define NONE 			1000
#define ACCEL_DATA 		2000
#define GYRO_DATA 		3000

#include "main.h"
#include "I2CSensor.h"

namespace OsprAi {

//Gyro Config
	//Bit 7 -> 1: Perform X Gyro Self-Test
	//Bit 6 -> 1: Perform Y Gyro Self-Test
	//Bit 5 -> 1: Perform Z Gyro Self-Test
	//Bit 4/3 -> 00: +250dps | 01: +500dps | 10: +1000dps | 11: +2000dps	Range measurement
	//Bit 2 -> 0: Reserved
	//Bit 1/0 -> 00: Fchoice
//Accelero Config
	//Bit 7 -> 1: Perform X Accel Self-Test
	//Bit 6 -> 1: Perform Y Accel Self-Test
	//Bit 5 -> 1: Perform Z Accel Self-Test
	//Bit 4/3 -> 00: +/-2G | 01: +/-4G | 10: +/-8G | 11: +/-16G	Range measurement
	//Bit 2 -> 0: Reserved
	//Bit 1/0 -> 00: Fchoice
//Wake UP du capteur
	//Bit 7 -> 1: Hard_Reset of all Registers
	//Bit 6 -> 1: Sleep Mode
	//Bit 5 -> 1: Alternate SLEEP <-> StandBy Mode at LP_ACCEL_ODR register Freq
	//Bit 4 -> 1: Low Powr Mode, pulling for getting gyro sample data
	//Bit 3 -> 1: Disable Temperature Sensor
	//Bit 2/1/0 -> 111: Stop clock, timing gen in reset | 110 and 000: Internal 20MHz Clock
	//				001 -> 101: Auto select best clock freq

class ImuManager : public I2CSensor {
private:
	float AccelRange;
	float GyroRange;
	vector<vector<float>> MeasurementData;
	uint8_t RawData[6];
	float LinAccelVect[3];
	float RotAccelVect[3];
	float Orientation[3];
	float MotionDirection[3];

	vector<float> ProcessAccelData(uint8_t *rawData) {
		int16_t signAccel[3];
		signAccel[0] = (((int16_t)rawData[0] << 8) | (rawData[1] << 0));
		signAccel[1] = (((int16_t)rawData[2] << 8) | (rawData[3] << 0));
		signAccel[2] = (((int16_t)rawData[4] << 8) + (rawData[5] << 0));
		float xAccel = (float)signAccel[0] / (pow((double)2, (double)16) / this->AccelRange);
		float yAccel = (float)signAccel[1] / (pow((double)2, (double)16) / this->AccelRange);
		float zAccel = (float)signAccel[2] / (pow((double)2, (double)16) / this->AccelRange);
		return {xAccel, yAccel, zAccel};
	}

	vector<float> ProcessGyroData(uint8_t *rawGyro) {
		int16_t signGyro[3];
		signGyro[0] = (((int16_t)rawGyro[0] << 8) | (rawGyro[1] << 0));
		signGyro[1] = (((int16_t)rawGyro[2] << 8) | (rawGyro[3] << 0));
		signGyro[2] = (((int16_t)rawGyro[4] << 8) + (rawGyro[5] << 0));

		float xRota = (float)signGyro[0] / (pow((double)2, (double)16) / this->GyroRange);
		float yRota = (float)signGyro[1] / (pow((double)2, (double)16) / this->GyroRange);
		float zRota = (float)signGyro[2] / (pow((double)2, (double)16) / this->GyroRange);
		return {xRota, yRota, zRota};
	}

public:
	ImuManager(vector<int> sensorAddresses, int samplesPerMes) : I2CSensor(sensorAddresses, samplesPerMes) {
		// TODO Auto-generated constructor stub

	}

	virtual ~ImuManager() {

	}

	HAL_StatusTypeDef SensorConfiguration() {
		HAL_StatusTypeDef status= HAL_ERROR;
		for (int i= 0; i < (int)SensorAddresses.size(); i++) {
			uint8_t configVal;
			uint8_t address;
			status= HAL_I2C_Mem_Read(I2cInterface, SensorAddresses.at(i), 0x75, 1, &address, 1, 1);
			status= HAL_I2C_IsDeviceReady(I2cInterface, SensorAddresses.at(i), 5, 1000);
			if (status != HAL_ERROR) {

				configVal= 0b00011000;
				if (configVal == 0b00000000)
					this->GyroRange = 250;
				else if (configVal == 0b00001000)
					this->GyroRange = 500;
				else if(configVal == 0b00010000)
					this->GyroRange = 1000;
				else if (configVal == 0b00011000)
					this->GyroRange= 2000;
				status = HAL_I2C_Mem_Write(I2cInterface, SensorAddresses.at(i), 0x1B, 1, &configVal, 1, 1);
				if (status == HAL_ERROR)
					return status;

				configVal= 0b00011000;
				if (configVal == 0b00000000)
					this->AccelRange = 4;
				else if (configVal == 0b00001000)
					this->AccelRange = 8;
				else if(configVal == 0b00010000)
					this->AccelRange = 16;
				else if (configVal == 0b00011000)
					this->AccelRange= 32;
				status = HAL_I2C_Mem_Write(I2cInterface, SensorAddresses.at(i), 0x1C, 1, &configVal, 1, 1);
				if (status == HAL_ERROR)
					return status;

				configVal= 0b00000000;
				status = HAL_I2C_Mem_Write(I2cInterface, SensorAddresses.at(i), 0x6B, 1, &configVal, 1, 1);
				if (status == HAL_ERROR)
					return status;
			}
		}
		return status;
	}

	HAL_StatusTypeDef TakeMeasurement() {
		HAL_StatusTypeDef status = HAL_OK;
		if (IsExecMeasurement == false) {
			IsExecMeasurement = true;
			//Acquisition of IMUs Data in Blocking mode
			vector<vector<float>> accelData;
			vector<vector<float>> gyroData;
			for (int i= 0; i < 3; i++){
				accelData.push_back({});
				gyroData.push_back({});
			}
			for (int i= 0; i < (int)SensorAddresses.size(); i++) {
				for(int j= 0; j < SamplesPerMes; j++) {
					status= HAL_I2C_Mem_Read(I2cInterface, SensorAddresses.at(i), 0x3B, 1, RawData, 6, 1);
					if (status == HAL_ERROR) {
						int a= 1;
					}
					vector<float> processedAccel= ProcessAccelData(RawData);
					float *test= processedAccel.data();
					for(int k= 0; k < (int)processedAccel.size(); k++) {
						accelData.at(k).push_back(processedAccel.at(k));
					}
					status= HAL_I2C_Mem_Read(I2cInterface, SensorAddresses.at(i), 0x43, 1, RawData, 6, 1);
					if (status == HAL_ERROR) {
						int a= 1;
					}
					vector<float> processedGyro= ProcessGyroData(RawData);
					float *test2= processedGyro.data();
					for(int k= 0; k < (int)processedGyro.size(); k++) {
						gyroData.at(k).push_back(processedGyro.at(k));
					}
				}
			}
			LinAccelVect[0]= Median(accelData[0]);
			LinAccelVect[1]= Median(accelData[1]);
			LinAccelVect[2]= Median(accelData[2]);
			RotAccelVect[0]= Median(gyroData[0]);
			RotAccelVect[1]= Median(gyroData[1]);
			RotAccelVect[2]=Median(gyroData[2]);

		}

		IsExecMeasurement = false;
		return status;
	}
};

} /* namespace Osprai */

#endif /* SRC_IMUMANAGER_H_ */
