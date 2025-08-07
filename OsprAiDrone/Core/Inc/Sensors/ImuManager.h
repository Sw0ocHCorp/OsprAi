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
#include "SensorManager.h"

namespace Osprai {

class ImuManager {
private:
	float AccelRange;
	float GyroRange;
	vector<vector<float>> MeasurementData;
	uint8_t RawData[6];
	float LinAccelVect[3];
	float RotAccelVect[3];
	float Orientation[3];
	float MotionDirection[3];
	float Timestep;
	vector<int> ImuAddresses;
	bool IsExecRoutine= false;
	int CurrentDataReceived= NONE;
	int LastDataReceived= NONE;
	int SamplesPerMes= 1;
	vector<int> RemainingSamples;
	I2C_HandleTypeDef *i2cInterface= nullptr;

	vector<float> ProcessAccelData(uint8_t *rawData) {
		int16_t signAccel[3];
		signAccel[0] = (((int16_t)rawData[0] << 8) | (rawData[1] << 0));
		signAccel[1] = (((int16_t)rawData[2] << 8) | (rawData[3] << 0));
		signAccel[2] = (((int16_t)rawData[4] << 8) + (rawData[5] << 0));

		signAccel[0] /= (pow((double)2, (double)16) / this->AccelRange);
		signAccel[1] /= (pow((double)2, (double)16) / this->AccelRange);
		signAccel[2] /= (pow((double)2, (double)16) / this->AccelRange);
		return {(float)signAccel[0], (float)signAccel[1], (float)signAccel[2]};
	}

	vector<float> ProcessGyroData(uint8_t *rawGyro) {
		int16_t signGyro[3];
		signGyro[0] = (((int16_t)rawGyro[0] << 8) | (rawGyro[1] << 0));
		signGyro[1] = (((int16_t)rawGyro[2] << 8) | (rawGyro[3] << 0));
		signGyro[2] = (((int16_t)rawGyro[4] << 8) + (rawGyro[5] << 0));

		signGyro[0] /= (pow((double)2, (double)16) / this->GyroRange);
		signGyro[1] /= (pow((double)2, (double)16) / this->GyroRange);
		signGyro[2] /= (pow((double)2, (double)16) / this->GyroRange);
		return {(float)signGyro[0], (float)signGyro[1], (float)signGyro[2]};
	}

public:
	ImuManager(int bufferSize) {
		// TODO Auto-generated constructor stub

	}

	virtual ~ImuManager() {

	}

	void setIMUAddresses(vector<int> imuAddresses) {
		ImuAddresses = imuAddresses;
		for (int i = 0; i < (int)imuAddresses.size(); i++) {
			RemainingSamples.push_back(SamplesPerMes);
		}
		for (int j= 0; j < 3; j++) {
			MeasurementData.push_back({});
		}
	}

	void setSamplesPerMeasurement(int nSample) {
		SamplesPerMes = nSample;
		for(int i= 0; i < (int)RemainingSamples.size(); i++) {
			RemainingSamples.at(i)= SamplesPerMes;
		}
	}

	HAL_StatusTypeDef SensorConfiguration(map<uint8_t, vector<uint8_t>> *configuration) {
		this->Timestep= HAL_GetTick()/(unsigned long)1000;
		HAL_StatusTypeDef status= HAL_ERROR;
		for (int i= 0; i < (int)ImuAddresses.size(); i++) {
			uint8_t configVal;
			uint8_t address;
			status= HAL_I2C_Mem_Read(this->i2cInterface, ImuAddresses.at(i), 0x75, 1, &address, 1, 1);
			status= HAL_I2C_IsDeviceReady(this->i2cInterface, ImuAddresses.at(i), 5, 1000);
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
					this->GyroRange = 250;
				else if (configVal == 0b00001000)
					this->GyroRange = 500;
				else if(configVal == 0b00010000)
					this->GyroRange = 1000;
				else if (configVal == 0b00011000)
					this->GyroRange= 2000;
				status = HAL_I2C_Mem_Write(this->i2cInterface, ImuAddresses.at(i), 0x1B, 1, &configVal, 1, 1);
				if (status == HAL_ERROR)
					return status;
				//Accelero Config
					//Bit 7 -> 1: Perform X Accel Self-Test
					//Bit 6 -> 1: Perform Y Accel Self-Test
					//Bit 5 -> 1: Perform Z Accel Self-Test
					//Bit 4/3 -> 00: +/-2G | 01: +/-4G | 10: +/-8G | 11: +/-16G	Range measurement
					//Bit 2 -> 0: Reserved
					//Bit 1/0 -> 00: Fchoice
				configVal= 0b00011000;
				if (configVal == 0b00000000)
					this->AccelRange = 4;
				else if (configVal == 0b00001000)
					this->AccelRange = 8;
				else if(configVal == 0b00010000)
					this->AccelRange = 16;
				else if (configVal == 0b00011000)
					this->AccelRange= 32;
				status = HAL_I2C_Mem_Write(this->i2cInterface, ImuAddresses.at(i), 0x1C, 1, &configVal, 1, 1);
				if (status == HAL_ERROR)
					return status;
				//Wake UP du capteur
					//Bit 7 -> 1: Hard_Reset of all Registers
					//Bit 6 -> 1: Sleep Mode
					//Bit 5 -> 1: Alternate SLEEP <-> StandBy Mode at LP_ACCEL_ODR register Freq
					//Bit 4 -> 1: Low Powr Mode, pulling for getting gyro sample data
					//Bit 3 -> 1: Disable Temperature Sensor
					//Bit 2/1/0 -> 111: Stop clock, timing gen in reset | 110 and 000: Internal 20MHz Clock
					//				001 -> 101: Auto select best clock freq
				configVal= 0b00000000;
				status = HAL_I2C_Mem_Write(this->i2cInterface, ImuAddresses.at(i), 0x6B, 1, &configVal, 1, 1);
				if (status == HAL_ERROR)
					return status;
			}
		}
		return status;
	}

	HAL_StatusTypeDef ExtractData(bool enableInterrupt) {
		HAL_StatusTypeDef status = HAL_OK;
		if (enableInterrupt && IsExecRoutine) {
			//IF in NONE Mode => ACCEL_MODE to launch the routine
			if (CurrentDataReceived == NONE)
				CurrentDataReceived = ACCEL_DATA;
			//Get Imu sensor to take the measurement
			int maxVal;
			int maxIndex;
			VecMax(RemainingSamples, &maxVal, &maxIndex);
			int imuAddress= ImuAddresses.at(maxIndex);
			//IF in ACCEL MODE => Process the incoming raw data(accel data)
			//	ONLY IF we are IN the measurement routine (last step NOT NONE)
			if (CurrentDataReceived == ACCEL_DATA && LastDataReceived != NONE) {
				vector<float> processedAccel= ProcessAccelData(RawData);
				for(int i= 0; i < (int)processedAccel.size(); i++) {
					MeasurementData.at(i).push_back(processedAccel.at(i));
				}
			}
			//IF in GYRO MODE => Process the incoming raw data(gyro data)
			if (CurrentDataReceived == GYRO_DATA) {
				vector<float> processedGyro= ProcessGyroData(RawData);
				for(int i= 0; i < (int)processedGyro.size(); i++) {
					MeasurementData.at(i).push_back(processedGyro.at(i));
				}
			}
			//IF all the measurements was taken
			//		ImuController need to be in NONE Mode to run another Routine
			if (maxVal == 0) {
				//Determine accel vector
				if (CurrentDataReceived == ACCEL_DATA) {
					CurrentDataReceived = GYRO_DATA;
					float med= Median(MeasurementData.at(0));
					LinAccelVect[0]= med;
					med= Median(MeasurementData.at(1));
					LinAccelVect[1]= med;
					med= Median(MeasurementData.at(2));
					LinAccelVect[2]= med;
				}
				//Determine gyro vector
				else if (CurrentDataReceived == GYRO_DATA) {
					CurrentDataReceived = NONE;
					IsExecRoutine = false;
					float med= Median(MeasurementData.at(0));
					RotAccelVect[0]= med;
					med= Median(MeasurementData.at(1));
					RotAccelVect[1]= med;
					med= Median(MeasurementData.at(2));
					RotAccelVect[2]= med;
				}
				//Reset remaining measurements for all imu sensors
				for (int i = 0; i < (int)ImuAddresses.size(); i++) {
					RemainingSamples[i] = SamplesPerMes;
				}
				//Clear the Measurements Buffer
				for (int j= 0; j < (int)MeasurementData.size(); j++) {
					MeasurementData[j]= {};
				}
			}
			else if (LastDataReceived != NONE)
				RemainingSamples[maxIndex]--;

			if (CurrentDataReceived == ACCEL_DATA)
				HAL_I2C_Mem_Read_IT(this->i2cInterface, imuAddress, 0x3B, 1, RawData, 6);
			if (CurrentDataReceived == GYRO_DATA)
				HAL_I2C_Mem_Read_IT(this->i2cInterface, imuAddress, 0x43, 1, RawData, 6);
			LastDataReceived= CurrentDataReceived;
		}
		if(enableInterrupt == false) {
			//Acquisition of IMUs Data in Blocking mode
			vector<vector<float>> accelData;
			vector<vector<float>> gyroData;
			for (int i= 0; i < 3; i++){
				accelData.push_back({});
				gyroData.push_back({});
			}
			for (int i= 0; i < (int)ImuAddresses.size(); i++) {
				for(int j= 0; j < SamplesPerMes; j++) {
					status= HAL_I2C_Mem_Read(this->i2cInterface, ImuAddresses.at(i), 0x3B, 1, RawData, 6, 1);
					vector<float> processedAccel= ProcessAccelData(RawData);
					for(int k= 0; k < (int)processedAccel.size(); k++) {
						accelData.at(k).push_back(processedAccel.at(k));
					}
					status= HAL_I2C_Mem_Read(this->i2cInterface, ImuAddresses.at(i), 0x43, 1, RawData, 6, 1);
					vector<float> processedGyro= ProcessGyroData(RawData);
					for(int k= 0; k < (int)processedGyro.size(); k++) {
						gyroData.at(k).push_back(processedGyro.at(k));
					}
				}
			}
			LinAccelVect[0]= Median(accelData.at(0));
			LinAccelVect[0]= Median(accelData.at(1));
			LinAccelVect[0]= Median(accelData.at(2));
			RotAccelVect[0]= Median(gyroData.at(0));
			RotAccelVect[1]= Median(gyroData.at(1));
			RotAccelVect[2]=Median(gyroData.at(2));
		}
		return status;
	}

	void LaunchRoutine() {
		if (IsExecRoutine == false) {
			IsExecRoutine= true;
			uint8_t address;
			HAL_I2C_Mem_Read_IT(this->i2cInterface, MPU1_SLAVE_ADDR, 0x75, 1, &address, 1);
		}
	}

	void setI2CInterface(I2C_HandleTypeDef *i2cInterface) { this->i2cInterface= i2cInterface; }
};

} /* namespace Osprai */

#endif /* SRC_IMUMANAGER_H_ */
