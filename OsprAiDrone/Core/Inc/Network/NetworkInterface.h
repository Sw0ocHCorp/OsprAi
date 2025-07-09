/*
 * NetworkInterface.h
 *
 *  Created on: Feb 16, 2025
 *      Author: Sw0ocH
 */

#ifndef NETWORKINTERFACE_H_
#define NETWORKINTERFACE_H_

#include "main.h"
#include "CircularBuffer.h"
#include "utils.h"
#include "Sensors/SensorManager.h"


namespace Osprai {

class NetworkInterface {
	private:
		I2C_HandleTypeDef *i2cBus= nullptr;
		UART_HandleTypeDef *uartBus = nullptr;
		CircularBuffer *rxBuffer;
		CircularBuffer *txBuffer;
		map<uint16_t, SensorManager *> sensorIds;
		uint8_t writeOpRegister;
		uint8_t readOpRegister;
		uint8_t lastByte;
		//------------ Standard Functions ------------

		//------------ Custom Functions ------------

	protected:
		vector<uint8_t> lastFrame;
		//------------ Standard Functions ------------
		HAL_StatusTypeDef ApplyRegistersConfig(uint16_t *sensorAddress, map<uint8_t, vector<uint8_t>> *configuration= nullptr);
		HAL_StatusTypeDef ApplyRegistersConfig(uint16_t *sensorAddress, map<uint8_t, uint8_t> *configuration= nullptr);
		//------------ Custom Functions ------------
		virtual HAL_StatusTypeDef ProcessFrame(uint8_t *ptrData)= 0;

	public:
		NetworkInterface(int bufferSize);
		virtual ~NetworkInterface();
		//------------ Standard Functions ------------
		void setSensorManagerIds(map<uint16_t, SensorManager *> ids);
		map<uint16_t, SensorManager *> getSensorManagerIds();
		void setI2CBus(I2C_HandleTypeDef *i2cBus);
		I2C_HandleTypeDef *getI2CBus();
		void setUARTBus(UART_HandleTypeDef *uartBus);
		UART_HandleTypeDef *getUARTBus();
		void setWriteOpRegister(uint8_t reg);
		uint8_t getWriteOpRegister();
		void setReadOpRegister(uint8_t reg);
		uint8_t getReadOpRegister();
		//Read Data from Register
		HAL_StatusTypeDef ReadSPIData(uint8_t registerAddress, uint8_t *outputData, int size);
		//Read Data in Register
		HAL_StatusTypeDef WriteSPIData(uint8_t registerAddress, uint8_t write_operation_reg, uint8_t data);
		HAL_StatusTypeDef WriteSPIData(uint8_t registerAddress, uint8_t write_operation_reg, uint8_t *data, int size);

		HAL_StatusTypeDef ExtractUARTData();

		//------------ Custom Functions ------------
		virtual HAL_StatusTypeDef PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, vector<uint8_t>> *configuration= nullptr)= 0;
		virtual HAL_StatusTypeDef PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, uint8_t> *configuration= nullptr)= 0;
		//virtual HAL_StatusTypeDef ReadIncomingData(uint8_t * data, bool enableInterrupt)= 0;
};

} /* namespace Osprai */

#endif /* NETWORKINTERFACE_H_ */
