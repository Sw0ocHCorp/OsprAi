#ifndef EVENTS_MANAGEMENT_HPP
#define EVENTS_MANAGEMENT_HPP
#include <Utils.hpp>
#include <functional>


template<typename T> class Observer {
    protected:
        std::function<void(T)> Callback;
    public:
        Observer() = default;
        virtual ~Observer() = default;

        void setCallback(std::function<void(T)> callback) {
            Callback = callback;
        }

        void respond(T data) {
            Callback(data);
        }
};

template<typename T> class Event { 
    private:
        vector<Observer<T> *> observers;
    public:
        void trigger(T eventData) {
            for (Observer<T> *observer : observers) {
                observer->respond(eventData);
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

#endif 