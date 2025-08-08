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
	class I2CSensor {
		protected:
			I2C_HandleTypeDef *I2cInterface;
			vector<int> SensorAddresses;
			int SamplesPerMes;
			bool IsExecMeasurement= false;

		public:
			I2CSensor(vector<int> sensorAddresses, int samplesPerMes) {
				SamplesPerMes= samplesPerMes;
				SensorAddresses = sensorAddresses;
			}

			~I2CSensor() {
			}

			void SetI2CInterface(I2C_HandleTypeDef *i2cInterface) {
				I2cInterface= i2cInterface;
			}

			virtual HAL_StatusTypeDef TakeMeasurement() = 0;
	};
}



#endif /* INC_SENSORS_I2CSENSOR_H_ */
