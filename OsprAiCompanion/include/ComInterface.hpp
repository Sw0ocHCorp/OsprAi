#ifndef COM_INTERFACE_HPP
#define COM_INTERFACE_HPP

#include <time.h>
#include <stdio.h>
#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include <pthread.h>
#include "EventsManagement.hpp"
#include "FrameParser.hpp"

using namespace std;

#define RC_INTERFACE        1
#define FC_INTERFACE        2

class ComInterface
{
    private:
        thread Task;
        
    protected:
        pthread_mutex_t Lock= PTHREAD_MUTEX_INITIALIZER;
        bool IsRunning = false;
        Event<WorldMap> DataReceivedEvent;
        Event<StaticVector<uint8_t, 500>> FrameReceivedEvent;
        FrameParser Parser;
        StaticVector<uint8_t, 500> OutputFrame;
        Observer<StaticVector<uint8_t, 500>> FrameToSendObserver;
        int Freq;
        int ID;


        virtual WorldMap dataFrameToWorldMap(StaticVector<StaticVector<float, 10>, 10> data)= 0;
        
        void runTask()  {
            bool isConnected = false;
            double cumulTime= 0;
            struct timespec start, end;
            double elapsedTime;
            while(this->IsRunning) {
                if (!isConnected) {
                    isConnected = connect();
                }
                else {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    StaticVector<uint8_t, 500> frame= listenForIncomingFrame();
                    clock_gettime(CLOCK_MONOTONIC, &end);
                    elapsedTime+= abs(end.tv_sec - start.tv_sec) * 1000.0
                                     + abs(end.tv_nsec - start.tv_nsec) / 1e6;      
                    if (frame.size() > 0){
                        //cout << "Frame size= " << frame.size() << " | Elapsed time= " << elapsedTime << " us" << endl;
                        StaticVector<StaticVector<float, 10>, 10> data= Parser.parseFrame(frame);
                        if (data.size() > 0) {
                            FrameReceivedEvent.trigger(frame);
                            WorldMap wMap= dataFrameToWorldMap(data);
                            DataReceivedEvent.trigger(wMap);
                            if (OutputFrame.size() > 0 && ID == FC_INTERFACE) {
                                pthread_mutex_lock(&Lock);
                                sendRawFrame(OutputFrame);
                                pthread_mutex_unlock(&Lock);
                                elapsedTime= 0.0;
                            }
                        }
                    }
                    if (elapsedTime > (1000.0 / Freq) && OutputFrame.size() > 0 && ID == RC_INTERFACE) {
                        pthread_mutex_lock(&Lock);
                        sendRawFrame(OutputFrame);
                        pthread_mutex_unlock(&Lock);
                        elapsedTime= 0.0;
                    }
                }
            }
        }

    public:
        ComInterface(FrameParser parser, int frequency, int id) {
            ID= id;
            Parser = parser;
            FrameToSendObserver.setCallback(std::bind(&ComInterface::sendRawFrame, this, std::placeholders::_1));
            Freq= frequency;
        }

        ~ComInterface(){
            stopTask();
        }

        void startTask() {
            this->IsRunning = true;
            Task = thread(&ComInterface::runTask, this);
        }

        virtual void stopTask() {
            this->IsRunning = false;
            if (Task.joinable()) {
                Task.join();
            }
        }

        virtual bool connect() = 0;

        virtual bool sendRawFrame(StaticVector<uint8_t, 500> frameData) = 0;

        virtual StaticVector<uint8_t, 500> listenForIncomingFrame()= 0;

        void enqueueNewFrame(StaticVector<uint8_t, 500> newFrame) {
            pthread_mutex_lock(&Lock);
            OutputFrame= newFrame;
            pthread_mutex_unlock(&Lock);
        }

        thread& getTask() {
            return Task;
        }

        void addFrameReceivedObserver(Observer<StaticVector<uint8_t, 500>> *observer) {
            FrameReceivedEvent.addObserver(observer);
        }

        void addDataReceivedObserver(Observer<WorldMap> *observer) {
            DataReceivedEvent.addObserver(observer);
        }
};

#endif