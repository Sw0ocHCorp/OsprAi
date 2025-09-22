#ifndef SERVOS_CONTROLLER_HPP
#define SERVOS_CONTROLLER_HPP

#include "EventsManagement.hpp"
#include <fstream>
#include "Utils.hpp"

#define MS_IN_NS    1000000

class ServosController : public ScheduledModule{
    protected:
        StaticVector<StaticVector<char, 50>, 4> PwmChannels;
        StaticVector<StaticVector<char, 50>, 4> PwmChips;
        std::shared_ptr<Observer<SetPoint>> SetPointObserver;
        std::shared_ptr<Observer<float>> ThetaObserver;
        bool IsRunning= false;
        float MinDCyRatio;
        float MaxDCyRatio;
        int CurrentDutyCycle;
        int AngleSetpoint;
        float AccelRatio;
        int Freq;
        int DegThetaRange;

    public:
        ServosController(StaticVector<StaticVector<char, 50>, 4> pwmChannelPaths, float minDutyCycle, float maxDutyCycle, float accelRatio, int frequency= 50, int thetaRange= 180) : ScheduledModule(50, false) {
            PwmChannels= pwmChannelPaths;
            MinDCyRatio= minDutyCycle;
            MaxDCyRatio= maxDutyCycle;
            AccelRatio= accelRatio;
            Freq= frequency;
            DegThetaRange= thetaRange;
            CurrentDutyCycle= MinDCyRatio * (1000/Freq)*MS_IN_NS;
            for (int i= 0; i < PwmChannels.size(); i++) {
                int endIndex= findPattern(PwmChannels[i].data(), PwmChannels[i].size(), (const char *)"/pwm", 4, true);
                vector<char> chip= PwmChannels[i].subVec(endIndex, false);
                StaticVector<char, 50> chipPath(chip.data(), chip.size());
                PwmChips.add(chipPath);
                chipPath.add("/export", 7);
                IsRunning= writeInLinuxFile(chipPath, StaticVector<char, 250> ("0", 1));
                if (IsRunning) {
                    string strVal= to_string((1000 / Freq)*MS_IN_NS);
                    StaticVector<char, 250> period(strVal.data(), strVal.size());
                    StaticVector<char, 50> channelPath(PwmChannels[i].data(), PwmChannels[i].size());
                    channelPath.add("/period", 7);
                    struct timespec start, end;
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                    double elapsedTime= abs(end.tv_sec - start.tv_sec) * 1000.0
                                     + abs(end.tv_nsec - start.tv_nsec) / 1e6; 
                      
                    while (elapsedTime < 100) {
                        clock_gettime(CLOCK_MONOTONIC, &end);
                        elapsedTime= abs(end.tv_sec - start.tv_sec) * 1000.0
                                     + abs(end.tv_nsec - start.tv_nsec) / 1e6; 
                    }
                    bool test= writeInLinuxFile(channelPath, period);
                    channelPath.clear();
                    channelPath.add(PwmChannels[i].data(), PwmChannels[i].size());
                    channelPath.add("/enable", 7);
                    AngleSetpoint= 90;
                    test= writeInLinuxFile(channelPath, StaticVector<char, 250> {'1'});
                    int a= 1;
                }
                SetPointObserver = std::make_shared<Observer<SetPoint>>();
                ThetaObserver = std::make_shared<Observer<float>>();
            }
        }

        ~ServosController() {

        }

        // /sys/class/pwm/pwmchip0/pwm0/period
        bool setAngleDutyCycle(int targetDegAngle) {
            if (IsRunning) {
                bool output= true;
                //WARNING: Duty Cycle is express in nanoseconds
                int minDutyCycle= MinDCyRatio * (1000/Freq)*MS_IN_NS;
                int maxDutyCycle= MaxDCyRatio * (1000/Freq)*MS_IN_NS;
                int targetDutyCycle= minDutyCycle + (targetDegAngle / (float)DegThetaRange) * (maxDutyCycle - minDutyCycle);
                if (CurrentDutyCycle > targetDutyCycle + AccelRatio*(maxDutyCycle - minDutyCycle)) {
                    CurrentDutyCycle -= AccelRatio*(maxDutyCycle - minDutyCycle);
                } else if(CurrentDutyCycle < targetDutyCycle - AccelRatio*(maxDutyCycle - minDutyCycle)) {
                    CurrentDutyCycle += AccelRatio*(maxDutyCycle - minDutyCycle);
                } else {
                    CurrentDutyCycle= targetDutyCycle;
                }
                for (int i= 0; i < PwmChannels.size(); i++) {
                    StaticVector<char, 50> channelPath(PwmChannels[i].data(), PwmChannels[i].size());
                    channelPath.add("/duty_cycle", 11);
                    string strVal= to_string(CurrentDutyCycle);
                    bool res= writeInLinuxFile(channelPath, StaticVector<char, 250> (strVal.c_str(), strVal.size()));
                    if (res == false && output == true) {
                        output= res;
                    }
                }
                return output;
            } else {
                return false;
            }
        }

        void execMainTask() {
            
        }



};

#endif