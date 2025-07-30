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

	class ServosController {
	private:
	public:
		ServosController();
		~ServosController();
	};

	class MotorsController {
		private:
			std::map<TIM_HandleTypeDef *, std::vector<unsigned int>> motorSources;
			MotorSetpoint CurrentSetpoint;
			bool isAutonomous= false;
			PIDController *pids;
		public:
			MotorsController() {

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

			void TestSetpoint(int value) {
				for(const auto & source : this->motorSources) {
					source.first->Instance->CCR1= value;
				}
			}

			void OnSetpointReceived(MotorSetpoint *setpoint) {
				CurrentSetpoint = *setpoint;
			}

			void UpdateMotorsCommand() {
				for(const auto & source : this->motorSources) {
					int pwmSetpoint= (abs(CurrentSetpoint.AngleSetpoint) / 3.15) * (MAX_PWM_VALUE/2);
					if (CurrentSetpoint.AngleSetpoint > 0) {
						pwmSetpoint += (MAX_PWM_VALUE/2);
					}
					source.first->Instance->CCR1= pwmSetpoint;
				}
			}
	};

} /* namespace Osprai */

#endif /* INC_MOTORSCONTROLLER_H_ */
