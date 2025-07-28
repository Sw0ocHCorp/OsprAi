/*
 * EventManagement.h
 *
 *  Created on: Jul 24, 2025
 *      Author: nclsr
 */

#ifndef INC_EVENTMANAGEMENT_H_
#define INC_EVENTMANAGEMENT_H_

#include <vector>

using namespace std;

template <typename T>
class Observer {
	public:
		virtual void Respond(T data)= 0;
		Observer();
		virtual ~Observer();
};

template <typename T>
class Event {
	private:
		vector<Observer<T> *> Observers;
	public:
		Event();
		virtual ~Event();
		void Trigger(T data) {
			for (Observer<T> *observer : Observers) {
				observer->Respond(data);
			}
		}
		void AddObserver(Observer<T> *obs) {
			Observers.push_back(obs);
		}
		void RemoveObserver(Observer<T> *obs) {
			Observers.erase(remove(Observers.begin(), Observers.end(), Observers), Observers.end());
		}
};

#endif /* INC_EVENTMANAGEMENT_H_ */
