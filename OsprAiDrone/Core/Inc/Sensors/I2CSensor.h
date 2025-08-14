/*
 * I2CSensor.h
 *
 *  Created on: Aug 8, 2025
 *      Author: nclsr
 */

#ifndef INC_SENSORS_I2CSENSOR_H_
#define INC_SENSORS_I2CSENSOR_H_

#include "main.h"
#include "utils.h"

namespace OsprAi {
	class I2CSensor : public ScheduledModule {
		protected:
			I2C_HandleTypeDef *I2cInterface;
			vector<int> SensorAddresses;
			vector<vector<float>> MeasurementsData;
			int SamplesPerMes;
			int SamplesTaken;
			int SensorIndex;
			bool IsRoutineFinished= false;

		public:
			I2CSensor(vector<int> sensorAddresses, int samplesPerMes) {
				SamplesPerMes= samplesPerMes;
				SensorAddresses = sensorAddresses;
				for (int i= 0; i < 3; i++) {
					MeasurementsData.push_back({});
				}
			}

			~I2CSensor() {
			}

			void SetI2CInterface(I2C_HandleTypeDef *i2cInterface) {
				I2cInterface= i2cInterface;
			}

			virtual void ExecMainTask()= 0;

			virtual void AskForMeasurement() = 0;

			virtual bool ProcessMeasurement(uint8_t sensorAddress, uint8_t memAddress) = 0;
	};
}



#endif /* INC_SENSORS_I2CSENSOR_H_ */
