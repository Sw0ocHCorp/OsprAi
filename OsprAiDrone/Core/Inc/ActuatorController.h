/*
 * MotorController.h
 *
 *  Created on: Jan 20, 2025
 *      Author: nclsr
 */

#ifndef INC_ACTUATORCONTROLLER_H_
#define INC_ACTUATORCONTROLLER_H_

#define MAX_PWM_VALUE 						65535
#define LL_PWM_SETPOINT						10
#define HL_SPEED_VEC_SETPOINT				20
#define HL_ANGLE_SETPOINT					21
#define HL_ANGLE_SPEED_VEC_SETPOINT			22



#include "main.h"
#include "utils.h"
#include "EventManagement.h"
#include <queue>

struct MotorSetpoint : Message {
	int SetpointType= 0;
	StaticVector<float, 10> PWMSetpoint;
	StaticVector<float, 3> SpeedVecSetpoint;
	float AngleSetpoint;
	StaticVector<float, 3> CurrentSpeedVec;
	StaticVector<float, 3> CurrentOrientation;
	StaticVector<float, 2> CurrentLocation;
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
			return true;
		}
	};

	class ActuatorController : public ScheduledModule {
		private:
		protected:

			StaticVector<StaticVector<uint32_t, 4>, 10> MotorChannels;
			StaticVector<TIM_HandleTypeDef *, 10> MotorSources;
			MotorSetpoint CurrentSetpoint;
			bool isAutonomous= false;
			PIDController *pids;
			std::shared_ptr<Observer<MotorSetpoint>> SetpointObserver;
			queue<MotorSetpoint> SetpointsBuffer;
			int BufferSize= 10;
			bool HaveReachSetpoint= true;
			int ArrValue;

		public:
			ActuatorController(StaticVector<TIM_HandleTypeDef *, 10> motorSources, StaticVector<StaticVector<uint32_t, 4>, 10> motorChannels,
																													int freq, bool isAutonomous) : ScheduledModule(freq, false) {
				MotorSources = motorSources;
				MotorChannels= motorChannels;
				SetpointObserver= std::make_shared<Observer<MotorSetpoint>>();
				SetpointObserver->setCallback(std::bind(&ActuatorController::OnSetpointReceived, this, std::placeholders::_1));
			}

			ActuatorController(StaticVector<TIM_HandleTypeDef *, 10> motorSources, StaticVector<StaticVector<uint32_t, 4>, 10> motorChannels,
																									int freq, int bufferSize, bool isAutonomous) : ScheduledModule(freq, false) {
				BufferSize = bufferSize;
				MotorSources = motorSources;
				MotorChannels= motorChannels;
			}

			virtual ~ActuatorController(){

			}

			HAL_StatusTypeDef InitController() {

				HAL_StatusTypeDef status= HAL_OK;
				for (int i= 0; i < MotorSources.size(); i++) {
					for (int j= 0; j < MotorChannels.size(); j++) {
						status= HAL_TIM_PWM_Start(MotorSources[i], MotorChannels[i][j]);
						ArrValue = MotorSources[i]->Instance->ARR;
						if (status != HAL_OK) {
							return status;
						}
					}
				}
				//pids= new PIDController[nMotors];
				this->isAutonomous= isAutonomous;
				return status;
			}

			void OnSetpointReceived(MotorSetpoint *setpoint) {
				while ((int)SetpointsBuffer.size() >= BufferSize)
					SetpointsBuffer.pop();
				SetpointsBuffer.push(*setpoint);
			}

			void ExecMainTask() {
				UpdateMotorsCommand();
			}

			virtual void UpdateMotorsCommand()= 0;

			void SetControlMode(bool isAutonomous) { this->isAutonomous= isAutonomous; }

			std::shared_ptr<Observer<MotorSetpoint>> GetSetpointObserver() {
				return SetpointObserver;
			}
	};

	class ServosController : public ActuatorController {
		private:
			int Accel;
			float MinDutyCylePercent;
			float MaxDutyCylePercent;
			float MaxAngle;
			int CurrentDCValue;
		public:
			ServosController(StaticVector<TIM_HandleTypeDef *, 10> motorSources, StaticVector<StaticVector<uint32_t, 4>, 10> motorChannels,
								int accel=1, float minDutyCylePercent= 0.025, float maxDutyCylePercent= 0.125, float maxAngle= 270, int bufferSize = 50) : ActuatorController(motorSources, motorChannels, 50, bufferSize) {
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
				for(int i= 0; i < MotorSources.size(); i++) {
					for (int j= 0; j < MotorChannels[i].size(); j++) {
						if (MotorChannels[i][j] == TIM_CHANNEL_1) {
							MotorSources[i]->Instance->CCR1= CurrentDCValue;
						}
						if (MotorChannels[i][j] == TIM_CHANNEL_2) {
							MotorSources[i]->Instance->CCR2= CurrentDCValue;
						}
						if (MotorChannels[i][j] == TIM_CHANNEL_3) {
							MotorSources[i]->Instance->CCR3= CurrentDCValue;
						}
						if (MotorChannels[i][j] == TIM_CHANNEL_4) {
							MotorSources[i]->Instance->CCR4= CurrentDCValue;
						}
					}
				}
			}
	};


} /* namespace Osprai */

#endif /* INC_ACTUATORCONTROLLER_H_ */
