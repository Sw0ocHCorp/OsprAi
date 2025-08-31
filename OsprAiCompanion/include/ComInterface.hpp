#ifndef COM_INTERFACE_HPP
#define COM_INTERFACE_HPP

#include <chrono>
#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include "EventsManagement.hpp"
#include "FrameParser.hpp"

using namespace std;

class ComInterface
{
    private:
        thread Task;
        
    protected:
        bool IsRunning = false;
        Event<WorldMap> DataReceivedEvent;
        Event<StaticVector<uint8_t, 500>> FrameReceivedEvent;
        FrameParser Parser;
        CircularBuffer<StaticVector<uint8_t, 500>, 25> FrameBuffer;
        Observer<StaticVector<uint8_t, 500>> FrameToSendObserver;

        virtual WorldMap dataFrameToWorldMap(StaticVector<StaticVector<float, 10>, 10> data)= 0;
        
        void runTask()  {
            bool isConnected = false;
            double cumulTime= 0;
            while(this->IsRunning) {
                if (!isConnected) {
                    isConnected = connect();
                }
                else {
                    /*struct timeval start;
                    struct timeval end;
                    gettimeofday(&start, NULL);*/
                    StaticVector<uint8_t, 500> frame= listenForIncomingFrame();
                    /*gettimeofday(&end, NULL);
                    double elapsedTime= end.tv_usec - start.tv_usec;*/
                    if (frame.size() > 0){
                        //cout << "Frame size= " << frame.size() << " | Elapsed time= " << elapsedTime << " us" << endl;
                        StaticVector<StaticVector<float, 10>, 10> data= Parser.parseFrame(frame);
                        if (data.size() > 0) {
                            FrameReceivedEvent.trigger(frame);
                            WorldMap wMap= dataFrameToWorldMap(data);
                            DataReceivedEvent.trigger(wMap);
                        }
                    }
                }
            }
        }

    public:
        ComInterface(FrameParser parser) {
            Parser = parser;
            FrameToSendObserver.setCallback(std::bind(&ComInterface::sendRawFrame, this, std::placeholders::_1));
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
            FrameBuffer.enqueue(newFrame);
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