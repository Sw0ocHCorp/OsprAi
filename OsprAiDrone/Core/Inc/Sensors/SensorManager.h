/*
 * SensorManager.h
 *
 *  Created on: Feb 14, 2025
 *      Author: Sw0ocH
 */

#ifndef SENSORMANAGER_H_
#define SENSORMANAGER_H_

#include "main.h"
#include "utils.h"
#include <vector>
#include<map>
#include "CircularBuffer.h"

using namespace std;

namespace Osprai {

class SensorManager {
	private:
		I2C_HandleTypeDef *i2cInterface= nullptr;
		UART_HandleTypeDef *uartInterface= nullptr;

		vector<uint8_t> lastFrame;
		CircularBuffer *rxBuffer;
		vector<uint8_t> rawSof;
		vector<uint8_t> newSof;
		char separator;
		map<uint16_t, uint16_t> parsingIds;
		//------------ Standard Functions ------------
		uint8_t ComputeCheckSum(vector<uint8_t> frame)  {
			uint8_t checkSum= frame.at(0);
			for(int i= 1; i < (int)frame.size(); i++) {
				checkSum |= frame.at(i);
			}
			return checkSum;
		}
		//------------ Custom Functions ------------
		virtual HAL_StatusTypeDef AnswerToRequest(vector<uint8_t> request)= 0;
		virtual HAL_StatusTypeDef ExtractData(bool enableInterrupt)= 0;

	public:
		SensorManager(int bufferSize) {
			// TODO Auto-generated constructor stub
			this->rxBuffer= new CircularBuffer(bufferSize);
		}

		virtual ~SensorManager() {

		}
		//------------ Standard Functions ------------
		HAL_StatusTypeDef UpdateData(bool enableInterrupt) {
			//Get data from Sensor
			ExtractData(enableInterrupt);
			//Give answers to all the requests in buffer;
			while(this->rxBuffer->getCurrentSize() > 0) {
				AnswerToRequest(this->rxBuffer->Dequeue());
			}
		}

		void setI2CInterface(I2C_HandleTypeDef *i2cInterface) { this->i2cInterface= i2cInterface; }
		I2C_HandleTypeDef *getI2CInterface() { return this->i2cInterface; }
		void setUARTInterface(UART_HandleTypeDef *uartInterface) { this->uartInterface= uartInterface; }
		UART_HandleTypeDef *getUARTInterface() { return this->uartInterface; }
		void setSeparator(char separator) { this->separator = separator; }
		char getSeparator() { return this->separator;}
		void setParsingIds(map<uint16_t, uint16_t> parsingIds) { this->parsingIds= parsingIds; }
		map<uint16_t, uint16_t> getParsingIds() {return this->parsingIds;}
		void setSof(vector<uint8_t> rawSof, vector<uint8_t> newSof) {
			this->rawSof= rawSof;
			this->newSof= newSof;
		}
		vector<uint8_t> getRawSof() {return this->rawSof;}
		vector<uint8_t> getNewSof() {return this->newSof;}
		HAL_StatusTypeDef StoreRequest(vector<uint8_t> frame) {
			return this->rxBuffer->Enqueue(frame);
		}
		//------------ Custom Functions ------------
		virtual HAL_StatusTypeDef SensorConfiguration(map<uint8_t, vector<uint8_t>> *configuration= nullptr)= 0;

};

} /* namespace Osprai */

#endif /* SENSORMANAGER_H_ */
