/*
 * MotorController.h
 *
 *  Created on: Jan 20, 2025
 *      Author: nclsr
 */

#ifndef INC_MOTORSCONTROLLER_H_
#define INC_MOTORSCONTROLLER_H_

#include "main.h"
#include <map>
#include <vector>

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
				pids= new PIDController[nMotors];
				this->isAutonomous= isAutonomous;
				return status;
			}

			void SetControlMode(bool isAutonomous) { this->isAutonomous= isAutonomous; }

			void ApplySetPoints(std::vector<int> setPoints)  {
				int nMotor= 0;
				for(const auto& timer: this->motorSources) {
					for (int i= 0; i < timer.second.size(); i++) {
						switch(timer.second[i]) {
							case TIM_CHANNEL_1:
								timer.first->Instance->CCR1= setPoints.at(nMotor);
								break;
							case TIM_CHANNEL_2:
								timer.first->Instance->CCR2= setPoints.at(nMotor);
								break;
							case TIM_CHANNEL_3:
								timer.first->Instance->CCR3= setPoints.at(nMotor);
								break;
							case TIM_CHANNEL_4:
								timer.first->Instance->CCR4= setPoints.at(nMotor);
								break;
						}
						nMotor++;
					}
				}
			}
			void UpdateMotorsSetPoints() {

			}
	};

} /* namespace Osprai */

#endif /* INC_MOTORSCONTROLLER_H_ */
