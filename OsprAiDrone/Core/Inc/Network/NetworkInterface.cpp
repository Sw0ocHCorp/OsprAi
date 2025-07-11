/*
 * CommunicationInterface.cpp
 *
 *  Created on: Feb 16, 2025
 *      Author: Sw0ocH
 */

#include <Network/NetworkInterface.h>

namespace Osprai {

NetworkInterface::NetworkInterface(int bufferSize) {
	// TODO Auto-generated constructor stub
	this->rxBuffer= new CircularBuffer(bufferSize);
	this->txBuffer= new CircularBuffer(bufferSize);
}

NetworkInterface::~NetworkInterface() {
	// TODO Auto-generated destructor stub
}

HAL_StatusTypeDef NetworkInterface::ApplyRegistersConfig(uint16_t *sensorAddress, map<uint8_t, uint8_t> *configuration) {
	if (configuration == nullptr) {
		return HAL_OK;
	}
	for (auto config = configuration->begin(); config != configuration->end(); ++config) {
		HAL_StatusTypeDef status = HAL_ERROR;
		if (getI2CBus() != nullptr) {
			status = HAL_I2C_Mem_Write(getI2CBus(), *sensorAddress, config->first, 1, &config->second, 1, 1000);
		}
		if (status == HAL_ERROR)
			return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef NetworkInterface::ApplyRegistersConfig(uint16_t *sensorAddress, map<uint8_t, vector<uint8_t>> *configuration) {
	if (configuration == nullptr) {
		return HAL_OK;
	}
	for (auto config = configuration->begin(); config != configuration->end(); ++config) {
		HAL_StatusTypeDef status = HAL_ERROR;
		if (getI2CBus() != nullptr) {
			for(int i= 0; i < (int)config->second.size(); i++) {
				uint8_t reg= config->first | (uint8_t)i;
				status = HAL_I2C_Mem_Write(getI2CBus(), *sensorAddress, reg, 1, &config->second.at(i), 1, 1000);
				if (status == HAL_ERROR)
					return HAL_ERROR;
			}
		}
		if (status == HAL_ERROR)
			return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef NetworkInterface::ExtractUARTData() {
	this->lastFrame.push_back(this->lastByte);
	//Using '\n' as EOF to simplify the detection of an entire frame
	if(this->lastByte == '\n') {

	}
}

//------------ GETERS / SETERS ------------
void NetworkInterface::setI2CBus(I2C_HandleTypeDef *i2cBus) {this->i2cBus= i2cBus;}
I2C_HandleTypeDef *NetworkInterface::getI2CBus() {return this->i2cBus;}
void NetworkInterface::setUARTBus(UART_HandleTypeDef *uartBus) {this->uartBus= uartBus; }
UART_HandleTypeDef *NetworkInterface::getUARTBus() {return this->uartBus;}
void NetworkInterface::setWriteOpRegister(uint8_t reg) {this->writeOpRegister= reg;}
uint8_t NetworkInterface::getWriteOpRegister() {return this->writeOpRegister;}
void NetworkInterface::setReadOpRegister(uint8_t reg) {this->readOpRegister= reg;}
uint8_t NetworkInterface::getReadOpRegister(){this->readOpRegister;}

void NetworkInterface::setSensorManagerIds(map<uint16_t, SensorManager *> ids) { this->sensorIds = ids;}
map<uint16_t, SensorManager *> NetworkInterface::getSensorManagerIds() {return this->sensorIds;}

} /* namespace Osprai */
