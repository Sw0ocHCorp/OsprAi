/*
 * UARTInterface.h
 *
 *  Created on: May 31, 2025
 *      Author: nclsr
 */

#ifndef INC_NETWORK_UARTINTERFACE_H_
#define INC_NETWORK_UARTINTERFACE_H_

#include "main.h"
#include <vector>

using namespace std;

namespace Osprai {
	class UARTInterface {
		private:
			uint8_t IncomingByte= '\n';
			string Frame;
			UART_HandleTypeDef *Bus;
			HAL_StatusTypeDef ProcessFrame(uint8_t *ptrData);
		public:
			UARTInterface(int bufferSize) {

			}

			virtual ~UARTInterface() {

			}

			void setBus(UART_HandleTypeDef *bus) {
				Bus = bus;
			}

			void ListeningForFrame(bool isBlocking= false) {
				//Blocking Way=
				//	Data reception -> Incoming data processing
				if (isBlocking) {
					HAL_UART_Receive(Bus, &this->IncomingByte, 1, 10);
				}
				//Receive End of frame data
				if (this->IncomingByte == '\n' && Frame.size() > 0) {
					Frame.clear();
				} else {
					Frame.push_back(this->IncomingByte);
					//Parse Frame
				}
				//Non Blocking Way=
				//	Data reception(method call from reception callback) -> Incoming data processing -> Listening for new data
				if (isBlocking == false) {
					HAL_UART_Receive_IT(Bus, &this->IncomingByte, 1);
				}
			}
	};
}
#endif /* INC_NETWORK_UARTINTERFACE_H_ */
