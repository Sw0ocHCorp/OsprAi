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

#define MAX_FRAME_SIZE 300
#define IMU_ID		1
#define BAROM_ID	2

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
			SPI_HandleTypeDef *Bus;
			FrameParser Parser;
			Event<MotorSetpoint> MotorSetpointReceivedEvent;
			std::shared_ptr<Observer<float>> ImuObserver;
			std::shared_ptr<Observer<float>> BarObserver;
			OsprAiState CurrentState;
			StaticVector<uint8_t, 1000> CompanionFrame;


		public:
			FlightController(int freq) : ScheduledModule(freq) {
				ImuObserver = std::make_shared<Observer<float>>();
				ImuObserver->setCallback(std::bind(&FlightController::ImuDataReceived, this, std::placeholders::_1));
				BarObserver = std::make_shared<Observer<float>>();
				BarObserver->setCallback(std::bind(&FlightController::AltitudeReceived, this, std::placeholders::_1));
			}

			virtual ~FlightController() {

			}

			void ImuDataReceived(float *imuData) {
				//HAL_GPIO_WritePin(GPIOA, LD2_Pin, GPIO_PIN_SET);
				//HAL_GPIO_TogglePin(GPIOA, LD2_Pin);
				CurrentState.LinearVelocity[0]= imuData[0];
				CurrentState.LinearVelocity[1]= imuData[1];
				CurrentState.LinearVelocity[2]= imuData[2];
				CurrentState.AngularVelocity[0]= imuData[3];
				CurrentState.AngularVelocity[0]= imuData[4];
				CurrentState.AngularVelocity[0]= imuData[5];
				CurrentState.Theta= imuData[6];
			}

			void AltitudeReceived(float *data) {
				//HAL_GPIO_WritePin(GPIOA, LD2_Pin, GPIO_PIN_RESET);
				//HAL_GPIO_TogglePin(GPIOA, LD2_Pin);
				CurrentState.Altitude= data[0];
			}

			void ExecMainTask() {
				StaticVector<uint8_t, 500> frame= Parser.EncodeFrame(StaticVector<StaticVector<uint8_t, 10>, 10> { StaticVector<uint8_t, 10> {0x00, 0x0F}, StaticVector<uint8_t, 10>{0x00, 0x10},
																																StaticVector<uint8_t, 10>{0x00, 0x11}, StaticVector<uint8_t, 10>{0x00, 0x12}
																															},
																					StaticVector<StaticVector<float,10>, 10> { StaticVector<float, 10> {CurrentState.LinearVelocity[0], CurrentState.LinearVelocity[1], CurrentState.LinearVelocity[2]},
																																	StaticVector<float, 10> {CurrentState.AngularVelocity[0], CurrentState.AngularVelocity[1], CurrentState.AngularVelocity[2]},
																																	StaticVector<float, 10> {CurrentState.Altitude}, StaticVector<float, 10> {CurrentState.Theta}
																															});
				frame.Add('\n');

				//HAL_GPIO_WritePin(GPIOA, LD2_Pin, GPIO_PIN_SET);
				HAL_StatusTypeDef status= HAL_SPI_TransmitReceive_IT(Bus, (uint8_t *)frame.data(), CompanionFrame.mutData(), frame.GetMaxSize());
				//HAL_StatusTypeDef status= HAL_SPI_Transmit_IT(Bus, (const uint8_t *)frame.GetData(), frame.GetSize());
				//HAL_StatusTypeDef status= HAL_UART_Transmit_IT(Bus, (uint8_t *)frame.GetData(), frame.GetSize());

			}

			void ListeningForFrame() {
				/*map<string, vector<float>> data;
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
				}*/
			}

			void ProcessReceivedFrame() {
				//HAL_GPIO_WritePin(GPIOA, LD2_Pin, GPIO_PIN_RESET);
				int a= 1;
			}

			void SetBus(SPI_HandleTypeDef *bus) {
				Bus = bus;
			}

			void SetParser(FrameParser parser) {
				Parser = parser;
			}

			shared_ptr<Observer<float>> GetDataObserver(int sensorID) {
				if (sensorID == IMU_ID)
					return ImuObserver;
				if (sensorID == BAROM_ID)
					return BarObserver;
				else
					return nullptr;
			}
	};
}

#endif /* INC_FLIGHTCONTROLLER_H_ */
