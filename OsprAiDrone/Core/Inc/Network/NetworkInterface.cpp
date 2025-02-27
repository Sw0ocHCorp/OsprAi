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
			else if (getSPIBus() != nullptr) {
				status= WriteSPIData(config->first, config->second, this->writeOpRegister);
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

			for(int i= 0; i < (int)config->second.size(); i++) {
				uint8_t reg= config->first | (uint8_t)i;
				HAL_StatusTypeDef status = HAL_ERROR;
				if (getI2CBus() != nullptr) {
					status = HAL_I2C_Mem_Write(getI2CBus(), *sensorAddress, reg, 1, &config->second.at(i), 1, 1000);
				}
				else if (getSPIBus() != nullptr) {
					status= WriteSPIData(reg, config->second.at(i), this->writeOpRegister);
				}
				if (status == HAL_ERROR)
					return HAL_ERROR;
			}

	}
	return HAL_OK;
}

//Read Data from Register
HAL_StatusTypeDef NetworkInterface::ReadSPIData(uint8_t registerAddress, uint8_t *outputValue) {
	//Set NSS to LOW to begin discussion
	HAL_GPIO_WritePin(SS_nRF24_GPIO_Port, SS_nRF24_Pin, GPIO_PIN_RESET);
	//Send register address
	HAL_StatusTypeDef status= HAL_SPI_Transmit(this->spiBus, &registerAddress, 1, 2000);
	//wait available data from Interface
	status= HAL_SPI_Receive(this->spiBus, outputValue, 1, 2000);
	//Set NSS to HIGH to end discussion
	HAL_GPIO_WritePin(SS_nRF24_GPIO_Port, SS_nRF24_Pin, GPIO_PIN_SET);
	return status;
}
//Write Data in Register
HAL_StatusTypeDef NetworkInterface::WriteSPIData(uint8_t registerAddress, uint8_t data, uint8_t write_operation_reg) {
	uint8_t reg_val[2] {registerAddress | write_operation_reg, data};
	//Set NSS to LOW to begin discussion
	HAL_GPIO_WritePin(SS_nRF24_GPIO_Port, SS_nRF24_Pin, GPIO_PIN_RESET);
	//Send register address to take action on it and value link with the register
	HAL_StatusTypeDef status= HAL_SPI_Transmit(this->spiBus, reg_val, 2, 2000);
	//Set NSS to HIGH to begin discussion
	HAL_GPIO_WritePin(SS_nRF24_GPIO_Port, SS_nRF24_Pin, GPIO_PIN_SET);
	return status;
}

//------------ GETERS / SETERS ------------
void NetworkInterface::setI2CBus(I2C_HandleTypeDef *i2cBus) {this->i2cBus= i2cBus;}
I2C_HandleTypeDef *NetworkInterface::getI2CBus() {return this->i2cBus;}
void NetworkInterface::setUARTBus(UART_HandleTypeDef *uartBus) {this->uartBus= uartBus; }
UART_HandleTypeDef *NetworkInterface::getUARTBus() {return this->uartBus;}
void NetworkInterface::setSPIBus(SPI_HandleTypeDef *spiBus) {this->spiBus = spiBus; }
SPI_HandleTypeDef *NetworkInterface::getSPIBus() {return this->spiBus; }
void NetworkInterface::setWriteRegister(uint8_t reg) {this->writeOpRegister= reg;}
uint8_t NetworkInterface::getWriteRegister() {return this->writeOpRegister;}

void NetworkInterface::setSensorManagerIds(map<uint16_t, SensorManager *> ids) { this->sensorIds = ids;}
map<uint16_t, SensorManager *> NetworkInterface::getSensorManagerIds() {return this->sensorIds;}

} /* namespace Osprai */
