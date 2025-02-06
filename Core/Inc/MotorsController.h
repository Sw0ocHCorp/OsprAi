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
	PIDController();
	~PIDController();
	void InitPID();
	float GetNewSetPoint();
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
	MotorsController();
	virtual ~MotorsController();
	HAL_StatusTypeDef InitController(std::map<TIM_HandleTypeDef *, std::vector<unsigned int>> motorSources, bool isAutonomous);
	void SetControlMode(bool isAutonomous);
	void ApplySetPoints(std::vector<int> setPoints);
	void UpdateMotorsSetPoints();
};

} /* namespace Osprai */

#endif /* INC_MOTORSCONTROLLER_H_ */
