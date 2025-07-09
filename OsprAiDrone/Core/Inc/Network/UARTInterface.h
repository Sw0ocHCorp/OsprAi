/*
 * UARTInterface.h
 *
 *  Created on: May 31, 2025
 *      Author: nclsr
 */

#ifndef INC_NETWORK_UARTINTERFACE_H_
#define INC_NETWORK_UARTINTERFACE_H_

#include "main.h"
#include "NetworkInterface.h"
namespace Osprai {
	class UARTInterface : public NetworkInterface {
		private:
			HAL_StatusTypeDef ProcessFrame(uint8_t *ptrData);
			HAL_StatusTypeDef PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, vector<uint8_t>> *configuration= nullptr);
			HAL_StatusTypeDef PerformCustomConfiguration(uint16_t *sensorAddress, map<uint8_t, uint8_t> *configuration= nullptr);
		public:
			UARTInterface(int bufferSize);
			virtual ~UARTInterface();

	};
}
#endif /* INC_NETWORK_UARTINTERFACE_H_ */
