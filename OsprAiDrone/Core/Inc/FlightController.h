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
#define SERVOS_ID 	3
#define MOTORS_ID	4

namespace OsprAi {
	struct OsprAiState {
		float LinearVelocity[3];
		float AngularVelocity[3];
		float Altitude;
		float Theta;

	};

	class FlightController : public ScheduledModule{
		private:
			UART_HandleTypeDef *Bus;
			FrameParser *Parser;
			Event<MotorSetpoint> MotorSetpointReceivedEvent;
			std::shared_ptr<Observer<float>> ImuObserver;
			std::shared_ptr<Observer<float>> BarObserver;
			OsprAiState CurrentState;
			StaticVector<uint8_t, 500> CompanionFrame;
			uint8_t ReceivedBytes[500];
			bool WaitingForFrame= false;

		public:
			FlightController(int freq) : ScheduledModule(freq, true) {
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
				StaticVector<uint8_t, 500> frame= Parser->EncodeFrame(StaticVector<StaticVector<uint8_t, 10>, 10> { StaticVector<uint8_t, 10> {0x00, 0x0F}, StaticVector<uint8_t, 10>{0x00, 0x10},
																																StaticVector<uint8_t, 10>{0x00, 0x11}, StaticVector<uint8_t, 10>{0x00, 0x12}
																															},
																					StaticVector<StaticVector<float,10>, 10> { StaticVector<float, 10> {CurrentState.LinearVelocity[0], CurrentState.LinearVelocity[1], CurrentState.LinearVelocity[2]},
																																	StaticVector<float, 10> {CurrentState.AngularVelocity[0], CurrentState.AngularVelocity[1], CurrentState.AngularVelocity[2]},
																																	StaticVector<float, 10> {CurrentState.Altitude}, StaticVector<float, 10> {CurrentState.Theta}
																															});

				HAL_UART_Transmit_IT(Bus, frame.data(), frame.size());

			}

			void ProcessIncomingFrame(StaticVector<StaticVector<float, 10>, 10> frameData) {
				StaticVector<StaticVector<char, 10>, 10> labels= Parser->GetParsingLabels();
				MotorSetpoint setpoint;
				//setpoint.PWMSetpoint= frameData[i];
				for(int i= 0; i < labels.size(); i++) {
					if (Equal(labels[i].data(), labels[i].size(), (const char *)"sticks", 6)) {
						if (setpoint.SetpointType == HL_SPEED_VEC_SETPOINT)
							setpoint.SetpointType= HL_ANGLE_SPEED_VEC_SETPOINT;
						else
							setpoint.SetpointType= HL_ANGLE_SETPOINT;
						setpoint.AngleSetpoint= atan2(frameData[i][1], frameData[i][0]);
					}
				}
				if (setpoint.SetpointType != 0) {
					MotorSetpointReceivedEvent.Trigger(&setpoint);
				}
			}

			void ProcessReceivedData(int dataSize) {
				//Let's assume the data is store in a specific array
				int startIndex= FindPattern(ReceivedBytes, dataSize, Parser->GetSOF().data(),
																		Parser->GetSOF().size());
				if (startIndex >= 0) {
					CompanionFrame.Add(ReceivedBytes, dataSize);
					for (int i= 0; i < startIndex; i++) {
						CompanionFrame.RemoveAt(0);
					}
					StaticVector<StaticVector<float, 10>, 10> data= Parser->ParseFrame(CompanionFrame, true);
					if (data.size() > 0) {
						HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
						ProcessIncomingFrame(data);
					}
					else
						HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
					CompanionFrame.Clear();
				} else {
					HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
				}
			}

			void ExecSecondTask() {
				HAL_UARTEx_ReceiveToIdle_IT(Bus, ReceivedBytes, sizeof(ReceivedBytes));
				/*HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);*/
			}

			void SetBus(UART_HandleTypeDef *bus) {
				Bus = bus;
			}

			void SetParser(FrameParser *parser) {
				Parser = parser;
			}

			void AttachMotorSetpointObserver(std::shared_ptr<Observer<MotorSetpoint>> obs) {
				MotorSetpointReceivedEvent.AddObserver(obs);
			}


			shared_ptr<Observer<float>> GetDataObserver(int deviceID) {
				if (deviceID == IMU_ID)
					return ImuObserver;
				if (deviceID == BAROM_ID)
					return BarObserver;
				else
					return nullptr;
			}

			UART_HandleTypeDef *GetBus() {
				return Bus;
			}

	};
}

#endif /* INC_FLIGHTCONTROLLER_H_ */
