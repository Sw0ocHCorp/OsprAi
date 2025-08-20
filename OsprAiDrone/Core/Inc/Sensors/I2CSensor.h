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
#include "EventManagement.h"

namespace OsprAi {
template <unsigned int NDataType>
	class I2CSensor : public ScheduledModule {
		protected:
			I2C_HandleTypeDef *I2cInterface;
			StaticVector<uint8_t, 10> SensorAddresses;
			StaticVector<StaticVector<float, 20>, NDataType> MeasurementsData;
			int SamplesPerMes;
			int SamplesTaken;
			int SensorIndex;
			bool IsRoutineFinished= false;
			Event<float> DataStreamingEvent;

		public:
			I2CSensor(int freq, StaticVector<uint8_t, 10> sensorAddresses, int samplesPerMes) :ScheduledModule(freq) {
				SensorAddresses = sensorAddresses;
				SamplesPerMes= samplesPerMes;
				for (int i= 0; i < MeasurementsData.GetMaxSize(); i++) {
					MeasurementsData.Add(StaticVector<float, 20>{});
				}
			}

			~I2CSensor() {
			}

			void SetI2CInterface(I2C_HandleTypeDef *i2cInterface) {
				I2cInterface= i2cInterface;
			}

			void AttachDataObserver(shared_ptr<Observer<float>> obs) {
				DataStreamingEvent.AddObserver(obs);
			}

			virtual void ExecMainTask()= 0;

			virtual void AskForMeasurement() = 0;

			virtual bool ProcessMeasurement(uint8_t sensorAddress, uint8_t memAddress) = 0;
	};
}



#endif /* INC_SENSORS_I2CSENSOR_H_ */
