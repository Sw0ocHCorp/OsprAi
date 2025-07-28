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
        Event<string> FrameReceivedEvent;
        FrameParser Parser;
        virtual void runTask() = 0;

    public:
        ComInterface(FrameParser parser) {
            Parser = parser;
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

        virtual bool sendFrame(string frame) = 0;

        thread& getTask() {
            return Task;
        }

        void addFrameReceivedObserver(Observer<string> *observer) {
            FrameReceivedEvent.addObserver(observer);
        }
};

#endif