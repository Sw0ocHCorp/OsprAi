/*
 * MotorController.h
 *
 *  Created on: Jan 20, 2025
 *      Author: nclsr
 */

#ifndef INC_ACTUATORCONTROLLER_H_
#define INC_ACTUATORCONTROLLER_H_

#define MAX_PWM_VALUE 65535

#include "main.h"
#include "utils.h"
#include "EventManagement.h"
#include <queue>

struct MotorSetpoint : Message {
	bool IsLLSetpoint;
	vector<float> PWMSetpoint;
	bool IsHLSetpoint;
	vector<float> SpeedVecSetpoint;
	float AngleSetpoint;
	vector<float> CurrentSpeedVec;
	vector<float> CurrentOrientation;
	vector<float> CurrentLocation;
};

namespace OsprAi {

	class PIDController {
	private:

	public:
		PIDController() {

		}

		~PIDController() {

		}

		void InitPID() {

		}

		float GetNewSetPoint() {

		}
	};

	class ActuatorController : public ScheduledModule {
		private:
		protected:
			std::map<TIM_HandleTypeDef *, std::vector<unsigned int>> motorSources;
			MotorSetpoint CurrentSetpoint;
			bool isAutonomous= false;
			PIDController *pids;
			queue<MotorSetpoint> SetpointsBuffer;
			int BufferSize;
			bool HaveReachSetpoint= true;
			int ArrValue;

		public:
			ActuatorController(int freq) : ScheduledModule(freq) {
			}

			ActuatorController(int freq, int bufferSize) : ScheduledModule(freq) {
				BufferSize = bufferSize;
			}

			virtual ~ActuatorController(){

			}

			HAL_StatusTypeDef InitController(std::map<TIM_HandleTypeDef *, std::vector<unsigned int>> motorSources, bool isAutonomous) {
				this->motorSources = motorSources;
				HAL_StatusTypeDef status= HAL_OK;
				int nMotors=0;
				for(const auto& timer: this->motorSources) {
					nMotors += timer.second.size();
					for (int i= 0; i < timer.second.size(); i++) {
						status= HAL_TIM_PWM_Start(timer.first, timer.second[i]);
						ArrValue = timer.first->Instance->ARR;
						if (status != HAL_OK) {
							return status;
						}
					}
				}
				//pids= new PIDController[nMotors];
				this->isAutonomous= isAutonomous;
				return status;
			}

			void SetControlMode(bool isAutonomous) { this->isAutonomous= isAutonomous; }

			void OnSetpointReceived(MotorSetpoint *setpoint) {
				while (SetpointsBuffer.size() >= BufferSize)
					SetpointsBuffer.pop();
				SetpointsBuffer.push(*setpoint);
			}

			void ExecMainTask() {
				uint32_t test= HAL_GetTick();
				if (HAL_GetTick() - StartTime >= 1000 / Freq) {
					StartTime = HAL_GetTick();
					UpdateMotorsCommand();
				}
			}

			virtual void UpdateMotorsCommand()= 0;
	};

	class ServosController : public ActuatorController {
		private:
			int Accel;
			float MinDutyCylePercent;
			float MaxDutyCylePercent;
			float MaxAngle;
			int CurrentDCValue;
		public:
			ServosController(int accel=1, float minDutyCylePercent= 0.025, float maxDutyCylePercent= 0.125, float maxAngle= 270, int bufferSize = 50) : ActuatorController(50, bufferSize) {
				Accel = accel;
				MinDutyCylePercent= minDutyCylePercent;
				MaxDutyCylePercent = maxDutyCylePercent;
				if (maxAngle > M_PI)
					MaxAngle= Deg2Rad(maxAngle, false);
			}

			~ServosController() { }

			void UpdateMotorsCommand() {
				float targetAngle= CurrentSetpoint.AngleSetpoint;
				if (CurrentSetpoint.AngleSetpoint < 0)
					targetAngle += 3.14;
				int rangeDC= ((int)(ArrValue*MaxDutyCylePercent) - (int)(ArrValue*MinDutyCylePercent)) * (M_PI / MaxAngle);
				int targetDCValue = (int)(ArrValue*MinDutyCylePercent) + (targetAngle / M_PI)*rangeDC;
				//If too close to target value => current value = target value
				if (abs(CurrentDCValue - targetDCValue) <= Accel) {
					CurrentDCValue = targetDCValue;
					HaveReachSetpoint = true;
				}
				//Need Acceleration
				if (CurrentDCValue < targetDCValue) {
					CurrentDCValue += Accel;
				}
				//Need Deceleration
				if(CurrentDCValue > targetDCValue) {
					CurrentDCValue -= Accel;
				}

				if (HaveReachSetpoint && SetpointsBuffer.size() > 0) {
					HaveReachSetpoint = false;
					CurrentSetpoint = SetpointsBuffer.front();
					SetpointsBuffer.pop();
				}
				for(const auto & source : this->motorSources) {
					for(int i= 0; i < source.second.size(); i++) {
						if (source.second[i] == TIM_CHANNEL_1) {
							source.first->Instance->CCR1= CurrentDCValue;
						}
						if (source.second[i] == TIM_CHANNEL_2) {
							source.first->Instance->CCR2= CurrentDCValue;
						}
						if (source.second[i] == TIM_CHANNEL_3) {
							source.first->Instance->CCR3= CurrentDCValue;
						}
						if (source.second[i] == TIM_CHANNEL_4) {
							source.first->Instance->CCR4= CurrentDCValue;
						}
					}
				}
			}
	};


} /* namespace Osprai */

#endif /* INC_ACTUATORCONTROLLER_H_ */
