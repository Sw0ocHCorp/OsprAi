/*
 * SensorManager.cpp
 *
 *  Created on: Feb 14, 2025
 *      Author: Sw0ocH
 */

#include <Sensors/SensorManager.h>

namespace Osprai {

SensorManager::SensorManager(int bufferSize) {
	// TODO Auto-generated constructor stub
	this->rxBuffer= new CircularBuffer(bufferSize);
}

SensorManager::~SensorManager() {
	// TODO Auto-generated destructor stub
}

HAL_StatusTypeDef SensorManager::StoreRequest(vector<uint8_t> frame) {
	return this->rxBuffer->Enqueue(frame);
}

HAL_StatusTypeDef SensorManager::UpdateData(bool enableInterrupt) {
	//Get data from Sensor
	ExtractData(enableInterrupt);
	//Give answers to all the requests in buffer;
	while(this->rxBuffer->getCurrentSize() > 0) {
		AnswerToRequest(this->rxBuffer->Dequeue());
	}
}

uint8_t SensorManager::ComputeCheckSum(vector<uint8_t> frame) {
	uint8_t checkSum= frame.at(0);
	for(int i= 1; i < (int)frame.size(); i++) {
		checkSum |= frame.at(i);
	}
	return checkSum;
}

//------------ GETERS / SETERS ------------
//Key of the Map -> Id to find in Frame
//New Id for the communication with the Wireless Controller
void SensorManager::setParsingIds(map<uint16_t, uint16_t> parsingIds) { this->parsingIds= parsingIds; }

map<uint16_t, uint16_t> SensorManager::getParsingIds() {return this->parsingIds;}

void SensorManager::setI2CInterface(I2C_HandleTypeDef *i2cInterface) { this->i2cInterface= i2cInterface; }

I2C_HandleTypeDef *SensorManager::getI2CInterface() { return this->i2cInterface; }

void SensorManager::setUARTInterface(UART_HandleTypeDef *uartInterface) { this->uartInterface= uartInterface; }

UART_HandleTypeDef *SensorManager::getUARTInterface() { return this->uartInterface; }

void SensorManager::setSof(vector<uint8_t> rawSof, vector<uint8_t> newSof) {
	this->rawSof= rawSof;
	this->newSof= newSof;
}

vector<uint8_t> SensorManager::getRawSof() {return this->rawSof;}

vector<uint8_t> SensorManager::getNewSof() {return this->newSof;}

void SensorManager::setSeparator(char separator) { this->separator = separator; }

char SensorManager::getSeparator() { return this->separator;}

} /* namespace Osprai */
