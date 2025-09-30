#ifndef LIDAR_MANAGER_HPP
#define LIDAR_MANAGER_HPP

#include "Utils.hpp"
#include "Actuators/ServosController.hpp"
#include "BasicInterfaces/I2CInterface.hpp"

class LidarManager : public ScheduledModule {
    private:
        ServosController *Servo;
        I2CInterface *TofInterface;
        StaticVector<uint8_t, 4> TofAddrs;
        bool RotateLeft= true;

    public:
        LidarManager(StaticVector<char, 25> i2cPortPath, StaticVector<uint8_t, 4> tofAddrs,
                        StaticVector<StaticVector<char, 50>, 2> servoPwmChannels, float minDutyCycle, float maxDutyCycle, 
                        float accelRatio, int servoFrequency= 50, int thetaRange= 180, int moduleFrequency= 50) : ScheduledModule(moduleFrequency, false) {
            TofAddrs= tofAddrs;
            TofInterface= new I2CInterface(i2cPortPath);
            StaticVector<StaticVector<char, 50>, 4> pwmChannels(servoPwmChannels.data(), servoPwmChannels.size());
            Servo= new ServosController(pwmChannels, minDutyCycle, maxDutyCycle, accelRatio, servoFrequency, thetaRange);
            
        }

        ~LidarManager() {

        }

        void execMainTask() override {
            updateServoSetpoint();
        }

        void updateServoSetpoint() {
            Servo->updateAngleDutyCycle();
            if (Servo->isSetpointReached()) {
                SetPoint setpoint;
                setpoint.SetpointType= SERVO_ANGLE_SETPOINT;
                StaticVector<int, 4> servoAngles= Servo->getCurrentAngles();
                for(int i= 0; i < servoAngles.size(); i++) {
                    if (servoAngles[i] > 170)
                        RotateLeft= false;
                    if (servoAngles[i] < 10) {
                        RotateLeft= true;
                    } 
                    if (RotateLeft) {
                        servoAngles[i]= 180;
                    } else {
                        servoAngles[i]= 0;
                    }
                }
                StaticVector<int, 10> angleSetpoints(servoAngles.data(), servoAngles.size());
                setpoint.ServosAngle= angleSetpoints;
                Servo->enqueueNewSetpoint(&setpoint);
            }
        }

};

#endif