/*
 * UARTInterface.h
 *
 *  Created on: May 31, 2025
 *      Author: nclsr
 */

#ifndef INC_NETWORK_UARTINTERFACE_H_
#define INC_NETWORK_UARTINTERFACE_H_

#include "main.h"
#include "FrameParser.h"
#include "EventManagement.h"
#include <vector>

using namespace std;

namespace Osprai {
	class UARTInterface {
		private:
			uint8_t IncomingByte= '\n';
			string Frame;
			UART_HandleTypeDef *Bus;
			FrameParser Parser;
			Event<MotorSetpoint> MotorSetpointReceivedEvent;
			HAL_StatusTypeDef ProcessFrame(uint8_t *ptrData);
		public:
			UARTInterface(int bufferSize) {

			}

			virtual ~UARTInterface() {

			}

			void setBus(UART_HandleTypeDef *bus) {
				Bus = bus;
			}

			void setParser(FrameParser parser) {
				Parser = parser;
			}

			void AttachMotorObserver(std::shared_ptr<MotorSetpointObserver> obs) {
				MotorSetpointReceivedEvent.AddObserver(obs);
			}

			void ListeningForFrame(bool isBlocking= false) {
				//Blocking Way=
				//	Data reception -> Incoming data processing
				map<string, vector<float>> data;
				if (isBlocking) {
					HAL_UART_Receive(Bus, &this->IncomingByte, 1, 10);
				} else {
					HAL_UART_Receive_IT(Bus, &this->IncomingByte, 1);
				}

				if (this->IncomingByte != '\n')
					Frame.push_back(this->IncomingByte);
				else if (this->IncomingByte == '\n') {
					if (Frame.size()> 5)
						data = Parser.parseFrame(Frame);
					int sz= data.size();
					if (data.count("sticks")) {
						MotorSetpoint setpoint;
						float test1= data["sticks"].at(1);
						float test0= data["sticks"].at(0);
						setpoint.AngleSetpoint = atan2(data["sticks"].at(1), data["sticks"].at(0));
						MotorSetpointReceivedEvent.Trigger(&setpoint);
					}
					Frame.clear();
				}

				/*if (Frame.size()> 1 && Frame.size() % 2 == 0)
					data = Parser.parseFrame(Frame);
				if (data.size() > 0) {
					Frame.clear();
				}*/
				//Receive End of frame data
				/*if (this->IncomingByte == '\n' && Frame.size() > 0) {

					Frame.clear();
				} else {
					Frame.push_back(this->IncomingByte);
					//Parse Frame
				}*/
				//Non Blocking Way=
				//	Data reception(method call from reception callback) -> Incoming data processing -> Listening for new data

			}
	};
}
#endif /* INC_NETWORK_UARTINTERFACE_H_ */
