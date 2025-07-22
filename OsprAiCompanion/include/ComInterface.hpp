#ifndef COM_INTERFACE_HPP
#define COM_INTERFACE_HPP

#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include <EventsManagement.hpp>

using namespace std;

class ComInterface
{
    private:
        thread Task;
    protected:
        bool IsRunning = false;
        Event<string> FrameReceivedEvent;

        virtual void runTask() = 0;

    public:
        ComInterface() {
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