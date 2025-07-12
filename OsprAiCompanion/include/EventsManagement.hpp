#ifndef EVENTS_MANAGEMENT_HPP
#define EVENTS_MANAGEMENT_HPP
#include <Utils.hpp>


template<typename T> class Observer {
    public:
        Observer() = default;
        virtual ~Observer() = default;
        virtual void onTriggerReceived(T eventData) = 0;
};

template<typename T> class Event { 
    private:
        vector<Observer<T> *> observers;
    public:
        void trigger(T eventData) {
            for (Observer<T> *observer : observers) {
                observer->onTriggerReceived(eventData);
            }
        }
        void addObserver(Observer<T> *observer) {
            observers.push_back(observer);
        }
        void removeObserver(Observer<T> *observer) {
            observers.erase(remove(observers.begin(), observers.end(), observer), observers.end());
        }
        virtual ~Event() = default;

};

class FrameReceivedObserver : public Observer<string> {
    public:
        void onTriggerReceived(string eventData) override {
            cout << "Frame received: " << eventData << endl;
            // Handle the received frame data
        }
};

#endif 