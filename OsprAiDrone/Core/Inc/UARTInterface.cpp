/*
 * UARTInterface.cpp
 *
 *  Created on: May 31, 2025
 *      Author: nclsr
 */

#include <Network/UARTInterface.h>


namespace Osprai {
	UARTInterface::UARTInterface(int bufferSize) : NetworkInterface(bufferSize) {
		// TODO Auto-generated constructor stub

	}

	UARTInterface::~UARTInterface() {
		// TODO Auto-generated destructor stub
	}

	HAL_StatusTypeDef UARTInterface::ProcessFrame(uint8_t *ptrData) {
		return HAL_OK;
	}

	HAL_StatusTypeDef UARTInterface::PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, vector<uint8_t>> *configuration) {
		return HAL_ERROR;
	}
	HAL_StatusTypeDef UARTInterface::PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, uint8_t> *configuration) {
		return HAL_ERROR;
	}
}

