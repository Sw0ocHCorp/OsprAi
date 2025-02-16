/*
 * SensorManager.h
 *
 *  Created on: Feb 14, 2025
 *      Author: nclsr
 */

#ifndef INC_SENSORMANAGER_H_
#define INC_SENSORMANAGER_H_

#include "main.h"
#include "utils.h"
#include <vector>
#include<map>
#include "CircularBuffer.h"

using namespace std;

namespace Osprai {

class SensorManager {
private:
	I2C_HandleTypeDef *i2cInterface;
	UART_HandleTypeDef *uartInterface;
	vector<uint8_t> lastFrame;
	CircularBuffer *rxBuffer;
	CircularBuffer *txBuffer;
	vector<uint8_t> rawSof;
	vector<uint8_t> newSof;
	char separator;
	map<uint16_t, uint16_t> parsingIds;
	//------------ Standard Functions ------------
	uint8_t ComputeCheckSum(vector<uint8_t> frame);
	//------------ Custom Functions ------------
	virtual HAL_StatusTypeDef AnswerToRequest(vector<uint8_t> request)= 0;
	virtual HAL_StatusTypeDef ExtractData(bool enableInterrupt)= 0;

public:
	SensorManager(int bufferSize);
	virtual ~SensorManager();
	//------------ Standard Functions ------------
	HAL_StatusTypeDef UpdateData(bool enableInterrupt);
	void setI2CInterface(I2C_HandleTypeDef *i2cInterface);
	I2C_HandleTypeDef *getI2CInterface();
	void setUARTInterface(UART_HandleTypeDef *i2cInterface);
	UART_HandleTypeDef *getUARTInterface();
	void setSeparator(char separator);
	char getSeparator();
	void setParsingIds(map<uint16_t, uint16_t> parsingIds);
	map<uint16_t, uint16_t> getParsingIds();
	void setSof(vector<uint8_t> rawSof, vector<uint8_t> newSof);
	vector<uint8_t> getRawSof();
	vector<uint8_t> getNewSof();
	HAL_StatusTypeDef StoreRequest(vector<uint8_t> frame);
	//------------ Custom Functions ------------
	virtual HAL_StatusTypeDef SensorConfiguration(map<uint8_t, vector<uint8_t>> *configuration= nullptr)= 0;

};

} /* namespace Osprai */

#endif /* INC_SENSORMANAGER_H_ */
