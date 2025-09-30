#ifndef SERVOS_CONTROLLER_HPP
#define SERVOS_CONTROLLER_HPP

#include "EventsManagement.hpp"
#include <fstream>
#include "Utils.hpp"

#define MS_IN_NS    1000000



class ServosController : public ScheduledModule{
    private:
        CircularBuffer<StaticVector<int, 4>, 50> AngleSetpointsQueue;
        pthread_mutex_t Lock= PTHREAD_MUTEX_INITIALIZER;
        bool SetpointReached= false;

    protected:
        StaticVector<StaticVector<char, 50>, 4> PwmChannels;
        StaticVector<StaticVector<char, 50>, 4> PwmChips;
        std::shared_ptr<Observer<SetPoint>> SetPointObserver;
        std::shared_ptr<Observer<float>> ThetaObserver;
        bool IsRunning= false;
        float MinDCyRatio;
        float MaxDCyRatio;
        StaticVector<int, 4> CurrentDutyCycles;
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
            for (int i= 0; i < PwmChannels.size(); i++) {
                CurrentDutyCycles.add(MinDCyRatio * (1000/Freq)*MS_IN_NS);
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
                    bool out= writeInLinuxFile(channelPath, period);
                    channelPath.clear();
                    channelPath.add(PwmChannels[i].data(), PwmChannels[i].size());
                    channelPath.add("/enable", 7);
                    bool out2= writeInLinuxFile(channelPath, StaticVector<char, 250> {'1'});
                    if (out == false || out2 == false) {
                        IsRunning= false;
                    } else {
                        StaticVector<int, 4> initialSetpoint;
                        for (int j= 0; j < PwmChannels.size(); j++) {
                            initialSetpoint.add(0);
                        }
                        AngleSetpointsQueue.enqueue(initialSetpoint);
                    }
                }
                SetPointObserver = std::make_shared<Observer<SetPoint>>();
                ThetaObserver = std::make_shared<Observer<float>>();
            }
        }

        ~ServosController() {

        }

        // /sys/class/pwm/pwmchip0/pwm0/period
        bool updateAngleDutyCycle() {
            if (IsRunning) {
                bool output= true;
                bool isReached= true;
                //WARNING: Duty Cycle is express in nanoseconds
                int minDutyCycle= MinDCyRatio * (1000/Freq)*MS_IN_NS;
                int maxDutyCycle= MaxDCyRatio * (1000/Freq)*MS_IN_NS;
                for (int i= 0; i < CurrentDutyCycles.size(); i++) {
                    int targetDutyCycle= minDutyCycle + (AngleSetpointsQueue.tail()[i] / (float)DegThetaRange) * (maxDutyCycle - minDutyCycle);
                    if (CurrentDutyCycles[i] > targetDutyCycle + AccelRatio*(maxDutyCycle - minDutyCycle)) {
                        CurrentDutyCycles[i] -= AccelRatio*(maxDutyCycle - minDutyCycle);
                        isReached= false;
                    } else if(CurrentDutyCycles[i] < targetDutyCycle - AccelRatio*(maxDutyCycle - minDutyCycle)) {
                        CurrentDutyCycles[i] += AccelRatio*(maxDutyCycle - minDutyCycle);
                        isReached= false;
                    } else {
                        CurrentDutyCycles[i]= targetDutyCycle;
                    }
                    StaticVector<char, 50> channelPath(PwmChannels[i].data(), PwmChannels[i].size());
                    channelPath.add("/duty_cycle", 11);
                    string strVal= to_string(CurrentDutyCycles[i]);
                    bool res= writeInLinuxFile(channelPath, StaticVector<char, 250> (strVal.c_str(), strVal.size()));
                    if (res == false && output == true) {
                        output= res;
                    }
                }
                if (isReached) {
                    pthread_mutex_lock(&Lock);
                    AngleSetpointsQueue.dequeue();
                    SetpointReached= true;
                    pthread_mutex_unlock(&Lock);
                } else if (SetpointReached) {
                    pthread_mutex_lock(&Lock);
                    SetpointReached= false;
                    pthread_mutex_unlock(&Lock);
                }
                return output;
            } else {
                return false;
            }
        }

        void execMainTask() {
            
        }

        void enqueueNewSetpoint(SetPoint *setpoint) {
            if (setpoint->SetpointType == SERVO_ANGLE_SETPOINT && setpoint->ServosAngle.size() <= 4) {
                pthread_mutex_lock(&Lock);
                StaticVector<int, 4> servosSetpoint(setpoint->ServosAngle.data(), setpoint->ServosAngle.size());
                AngleSetpointsQueue.enqueue(servosSetpoint);
                pthread_mutex_unlock(&Lock);
            }
        }

        StaticVector<int, 4> getCurrentAngles() {
            StaticVector<int, 4> angles;
            int minDutyCycle= MinDCyRatio * (1000/Freq)*MS_IN_NS;
            int maxDutyCycle= MaxDCyRatio * (1000/Freq)*MS_IN_NS;
            for (int i= 0; i < CurrentDutyCycles.size(); i++) {
                int angle= (CurrentDutyCycles[i]-minDutyCycle) * DegThetaRange / (maxDutyCycle - minDutyCycle);
                angles.add(angle);
            }
            return angles;
        }

        int getFreq() {
            return Freq;
        }

        bool isSetpointReached() {
            return SetpointReached;
        }



};

#endif