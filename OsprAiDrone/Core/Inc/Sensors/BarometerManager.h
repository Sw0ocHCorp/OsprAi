#ifndef SRC_BAROMETERMANAGER_H_
#define SRC_BAROMETERMANAGER_H_

#define BARO1_SLAVE_ADDR 0x76 << 1
#define BARO2_SLAVE_ADDR 0x77 << 1
#define BARO_TIMER_DELAY	5

#include "main.h"
#include "I2CSensor.h"

namespace OsprAi {
	//WARNING:
	//	For BME280, Calibration data are Little Endian, it means LSB first
	class BarometerManager : public I2CSensor<2> {
	private:
		float Pressure;
		float Temperature;
		uint16_t P1ConfigData;
		uint16_t T1ConfigData;
		int16_t T2xConfigData[2];
		int16_t P2xConfigData[8];
		bool IsRoutineFinished = false;
		bool MeasurementCalled= false;
		uint8_t RawData[6];
		uint8_t DevStatus;
		//Temperature Oversampling(*2) & Presure Oversampling(*8) & mode(FORCED= 11)
		uint8_t ModeValue= 2 << 5 | 4 << 2 | 1;

		float ProcessTemperature(int32_t rawTemp, int32_t *tFine) {
			//4 first bits of xlsb is useless because set to 0
			rawTemp >>= 4;
			int32_t var1= (int32_t)((rawTemp / 8) - (int32_t)(T1ConfigData * 2));
			var1= (var1 * (int32_t)(T2xConfigData[0])) / 2048;
			int32_t var2= (int32_t)((rawTemp / 16) - (int32_t)(T1ConfigData));
			var2= (((var2 * var2) / 4096) * (int32_t)T2xConfigData[1]) / 16384;
			*tFine = var1 + var2;
			int32_t temp= ((*tFine) * 5 + 128) / 256;
			return (float)temp / 100.0;
		}

		float ProcessPressure(int32_t rawPressure, int32_t tFine) {
			rawPressure >>= 4;
			int64_t var1= (int64_t)tFine - 128000;
			int64_t var2= var1*var1 * (int64_t)P2xConfigData[4];
			var2 += (var1 * (int64_t)(P2xConfigData[3])) * 131072;
			var2 += (int64_t)(P2xConfigData[2]) * 34359738368;
			var1= ((var1*var1 * (int64_t)(P2xConfigData[1])) / 256) +
						(var1 * (int64_t)(P2xConfigData[0]) * 4096);
			var1 = (var1 + 140737488355328) * (int64_t)P1ConfigData / 8589934592;
			if (var1 == 0) {
				return 0; // avoid exception caused by division by zero
			}
			int64_t var4= 1048576 - rawPressure;
			var4= (((var4 * 2147483648) - var2) * 3125) / var1;
			var1 = ((int64_t)P2xConfigData[7] * (var4 / 8192) * (var4 / 8192)) / 33554432;
			var2 = ((int64_t)P2xConfigData[6] * var4) / 524288;
			var4 = ((var4 + var1 + var2) / 256) + ((int64_t)P2xConfigData[5] * 16);
			float pressure = var4 / 256.0;

			return pressure;
		}

	public:
		BarometerManager(int taskFreq, StaticVector<uint8_t, 10> sensorAddresses, int samplesPerMes) : I2CSensor<2>(taskFreq*2, sensorAddresses, samplesPerMes) {
			// TODO Auto-generated constructor stub
		}

		virtual ~BarometerManager() {

		}

		HAL_StatusTypeDef SensorConfiguration() {
			HAL_StatusTypeDef status= HAL_ERROR;
			for (int i= 0; i < this->SensorAddresses.GetSize(); i++) {
				//Get Sensor ID:
				//	0x56-0x58 => BMP 280
				//	0x60 => BME 280
				//	0x61 => BME 680
				uint8_t sensorId;
				status= HAL_I2C_Mem_Read(this->I2cInterface, this->SensorAddresses[i], 0xD0, 1, &sensorId, 1, 1);
				//IF the Mem_Read is succesfull(data available and sensor connected) AND sensor ID is different that 0xFF (means if a Sensor is detected)
				if (status == HAL_OK && sensorId != 0xFF) {
					//Execute Soft RESET
					uint8_t resetReg= 0xB6;
					status= HAL_I2C_Mem_Write(this->I2cInterface, this->SensorAddresses[i], 0xE0, 1, &resetReg, 1, 1);
					//Check If Calibration data is available (Sensor finished collect calibration data)
					uint8_t devStatus;
					status= HAL_I2C_Mem_Read(this->I2cInterface, this->SensorAddresses[i], 0xF3, 1, &devStatus, 1, 1);
					//IF not, loop to wait for those data
					while (status == HAL_OK && devStatus >> 4 == 1) {
						HAL_Delay(1);
						status= HAL_I2C_Mem_Read(this->I2cInterface, this->SensorAddresses[i], 0xF3, 1, &devStatus, 1, 1);
					}
					HAL_Delay(10);
					//Save Calibration Data
					int j= 0;
					for (uint8_t configReg= 0x88; configReg <= 0x9E; configReg += 0x02) {
						uint8_t calibrationData[2];
						status = HAL_I2C_Mem_Read(this->I2cInterface, this->SensorAddresses[i], configReg, 1,calibrationData, 2, 1);
						if (configReg == 0x88) {
							T1ConfigData = (int16_t)((unsigned)calibrationData[1] << 8 | (unsigned)calibrationData[0] << 0);
							j= 0;
						} else if(configReg == 0x8E) {
							P1ConfigData = (int16_t)((unsigned)calibrationData[1] << 8 | (unsigned)calibrationData[0] << 0);
							j= 0;
						} else if (configReg < 0x8E) {
							T2xConfigData[j] = calibrationData[1] << 8 | calibrationData[0] << 0;
							j++;
						} else {
							P2xConfigData[j] = calibrationData[1] << 8 | calibrationData[0] << 0;
							j++;
						}
					}
					//Set Sensor in SLEEP MODE to configure sampling
					uint8_t mode= 0x00;
					status= HAL_I2C_Mem_Write(this->I2cInterface, this->SensorAddresses[i], 0xF4, 1, &mode, 1, 1);
					if (status == HAL_OK) {
						//Set Temperature Oversampling(*2) & Presure Oversampling(*8) & mode(FORCED= 11)
						//	=> Target use case is (Indoor) Navigation (We need fast & synchronizable measurements)
						status= HAL_I2C_Mem_Write(this->I2cInterface, this->SensorAddresses[i], 0xF4, 1, &ModeValue, 1, 1);
						//Set Sensor Hardware config
						//	Standby Time(time between 2 Measurements)= 10ms (Not important when use Sensor in FORCED MODE)
						// 	IIR Filter(Reduce Noise)= 16
						uint8_t configValue =  7 << 2;
						status= HAL_I2C_Mem_Write(this->I2cInterface, this->SensorAddresses[i], 0xF5, 1, &configValue, 1, 1);

					}
				}
			}
			return status;
		}

		bool IsDataAvailable() {
			if ((DevStatus & 1) == 0) {
				return true;
			}
			else {
				return false;
			}
		}

		void ExecMainTask() {
			if (this->MeasurementCalled == false)
				LaunchMeasurementRoutine();
			else
				AskForMeasurement();

		}

		void LaunchMeasurementRoutine() {
			if (this->MeasurementCalled == false) {
				this->MeasurementCalled= true;
			}
			HAL_I2C_Mem_Write(this->I2cInterface, this->SensorAddresses[this->SensorIndex], 0xF4, 1, &this->ModeValue, 1, 1);
		}

		void CheckIfDataAvailable() {
			HAL_I2C_Mem_Read_IT(this->I2cInterface, this->SensorAddresses[this->SensorIndex], 0xF3, 1, &this->DevStatus, 1);
		}

		void AskForMeasurement() {
			HAL_I2C_Mem_Read_IT(this->I2cInterface, this->SensorAddresses[this->SensorIndex], 0xF7, 1, this->RawData, 6);
		}

		bool ProcessMeasurement(uint8_t devAddr, uint8_t regReaded) {
			this->SamplesTaken++;
			//Calibration data are Little Endian, it means LSB first
			int32_t rawPres = (this->RawData[0] << 16) | (this->RawData[1] << 8) | (this->RawData[2] << 0);
			int32_t rawTemp = (this->RawData[3] << 16) | (this->RawData[4] << 8) | (this->RawData[5] << 0);
			int32_t tFine;
			float temp= ProcessTemperature(rawTemp, &tFine);
			float pres= ProcessPressure(rawPres, tFine) / 100.0;
			this->MeasurementsData[0].Add(temp);
			this->MeasurementsData[1].Add(pres);
			if (this->SamplesTaken >= this->SamplesPerMes) {
				this->SamplesTaken= 0;
				this->SensorIndex++;
				if (this->SensorIndex >= (int)this->SensorAddresses.GetSize()) {
					this->SensorIndex= 0;
					this->Temperature= Median((float *)this->MeasurementsData[0].GetData(), this->MeasurementsData[0].GetSize());
					this->Pressure= Median((float *)this->MeasurementsData[0].GetData(), this->MeasurementsData[0].GetSize());
					for (int i = 0; i < (int)this->MeasurementsData.GetSize(); i++) {
						this->MeasurementsData[i].Clear();
					}
				}
			}
			float data[2] {Pressure, Temperature};
			DataStreamingEvent.Trigger(data);
			this->MeasurementCalled= false;
			this->DevStatus= 0;
			return true;
		}
	};

} /* namespace Osprai */

#endif
