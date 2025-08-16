/*
 * FlightController.h
 *
 *  Created on: Aug 15, 2025
 *      Author: nclsr
 */

#ifndef INC_FLIGHTCONTROLLER_H_
#define INC_FLIGHTCONTROLLER_H_

#include "main.h"
#include "FrameParser.h"
#include "EventManagement.h"
#include "ActuatorController.h"



namespace OsprAi {
	struct OsprAiState {
		float LinearVelocity[3];
		float AngularVelocity[3];
		float Altitude;
		float Theta;

	};

	class FlightController : public ScheduledModule{
		private:
			uint8_t IncomingByte= '\n';
			string Frame;
			UART_HandleTypeDef *Bus;
			FrameParser Parser;
			Event<MotorSetpoint> MotorSetpointReceivedEvent;
			std::shared_ptr<Observer<float>> ImuObserver;
			std::shared_ptr<Observer<float>> BarObserver;
			OsprAiState CurrentState;


		public:
			FlightController(int freq) : ScheduledModule(freq) {
				ImuObserver = std::make_shared<Observer<float>>();
				ImuObserver->setCallback(std::bind(&FlightController::ImuDataReceived, this, std::placeholders::_1));
				BarObserver = std::make_shared<Observer<float>>();
				BarObserver->setCallback(std::bind(&FlightController::AltitudeReceived, this, std::placeholders::_1));
			}

			virtual ~FlightController() {

			}

			void SetBus(UART_HandleTypeDef *bus) {
				Bus = bus;
			}

			void SetParser(FrameParser parser) {
				Parser = parser;
			}

			void ImuDataReceived(float *imuData) {
				CurrentState.LinearVelocity[0]= imuData[0];
				CurrentState.LinearVelocity[1]= imuData[1];
				CurrentState.LinearVelocity[2]= imuData[2];
				CurrentState.AngularVelocity[0]= imuData[3];
				CurrentState.AngularVelocity[0]= imuData[4];
				CurrentState.AngularVelocity[0]= imuData[5];
				CurrentState.Theta= imuData[6];
			}

			void AltitudeReceived(float *altitude) {
				CurrentState.Altitude= *altitude;
			}

			void ExecMainTask() {
				if (HAL_GetTick() - StartTime > 1000 / Freq) {
					string frame= Parser.EncodeFrame({{"000F", {CurrentState.LinearVelocity[0], CurrentState.LinearVelocity[1], CurrentState.LinearVelocity[2]}},
																{"0010", {CurrentState.AngularVelocity[0], CurrentState.AngularVelocity[1], CurrentState.AngularVelocity[2]}},
																{"0011", {CurrentState.Altitude}},
																{"0012", {CurrentState.Theta}}});
					frame.push_back('\n');
					HAL_StatusTypeDef status= HAL_UART_Transmit_IT(Bus, (uint8_t *)frame.data(), frame.size());
					int a= 1;
				}
			}

			void ListeningForFrame() {
				//Blocking Way=
				//	Data reception -> Incoming data processing
				map<string, vector<float>> data;
				HAL_UART_Receive_IT(Bus, &this->IncomingByte, 1);
					if (this->IncomingByte != '\n')
					Frame.push_back(this->IncomingByte);
				else if (this->IncomingByte == '\n') {
					if (Frame.size()> 5)
						data = Parser.ParseFrame(Frame);
					if (data.count("sticks")) {
						MotorSetpoint setpoint;
						setpoint.AngleSetpoint = atan2(data["sticks"].at(1), data["sticks"].at(0));
						//MotorSetpointReceivedEvent.Trigger(&setpoint);
					}
					Frame.clear();
				}
			}
	};
}

#endif /* INC_FLIGHTCONTROLLER_H_ */
