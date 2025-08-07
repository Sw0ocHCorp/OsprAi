/*
 * MotorController.h
 *
 *  Created on: Jan 20, 2025
 *      Author: nclsr
 */

#ifndef INC_MOTORSCONTROLLER_H_
#define INC_MOTORSCONTROLLER_H_

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

class MotorSetpointObserver : public Observer<MotorSetpoint> {
	public:
		MotorSetpointObserver() : Observer() {

		}

		~MotorSetpointObserver() {

		}

		void Respond(MotorSetpoint *setpoint) {
			Callback(setpoint);
		}
};

namespace Osprai {

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

	class MotorsController {
		private:
		protected:
			std::map<TIM_HandleTypeDef *, std::vector<unsigned int>> motorSources;
			MotorSetpoint CurrentSetpoint;
			bool isAutonomous= false;
			PIDController *pids;
			queue<MotorSetpoint> SetpointsBuffer;
			int BufferSize;
			bool HaveReachSetpoint= true;
		public:
			MotorsController() {
			}

			MotorsController(int bufferSize) {
				BufferSize = bufferSize;
			}

			virtual ~MotorsController(){

			}

			HAL_StatusTypeDef InitController(std::map<TIM_HandleTypeDef *, std::vector<unsigned int>> motorSources, bool isAutonomous) {
				this->motorSources = motorSources;
				HAL_StatusTypeDef status= HAL_OK;
				int nMotors=0;
				for(const auto& timer: this->motorSources) {
					nMotors += timer.second.size();
					for (int i= 0; i < timer.second.size(); i++) {
						status= HAL_TIM_PWM_Start(timer.first, timer.second[i]);
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

			virtual void UpdateMotorsCommand()= 0;
	};

	class ServosController : public MotorsController {
		private:
			int Accel;
			int MinDutyCyleValue;
			int MaxDutyCyleValue;
			float MaxAngle;
			int CurrentDCValue;
		public:
			ServosController(int accel=1, int minDutyCyleValue= 250, int maxDutyCycleValue= 1250, float maxAngle= 270, int bufferSize = 50) : MotorsController(bufferSize) {
				Accel = accel;
				MinDutyCyleValue= minDutyCyleValue;
				MaxDutyCyleValue = maxDutyCycleValue;
				if (maxAngle > M_PI)
					MaxAngle= Deg2Rad(maxAngle, false);
				CurrentDCValue= minDutyCyleValue;
			}

			~ServosController() { }

			void UpdateMotorsCommand() {
				float targetAngle= CurrentSetpoint.AngleSetpoint;
				if (CurrentSetpoint.AngleSetpoint < 0)
					targetAngle += 3.14;
				int rangeDC= (MaxDutyCyleValue - MinDutyCyleValue) * (M_PI / MaxAngle);
				int targetDCValue = MinDutyCyleValue + (targetAngle / M_PI)*rangeDC;
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
					CurrentSetpoint = SetpointsBuffer.back();
					SetpointsBuffer.pop();
				}
				for(const auto & source : this->motorSources)
					source.first->Instance->CCR1= CurrentDCValue;
			}
	};


} /* namespace Osprai */

#endif /* INC_MOTORSCONTROLLER_H_ */
