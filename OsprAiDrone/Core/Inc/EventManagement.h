/*
 * EventManagement.h
 *
 *  Created on: Jul 24, 2025
 *      Author: nclsr
 */

#ifndef INC_EVENTMANAGEMENT_H_
#define INC_EVENTMANAGEMENT_H_

#include "main.h"
#include <vector>
#include <functional>

using namespace std;

struct Message {
};

template <typename T>
class Observer {
	protected:
		function<void(T *)> Callback;
	public:
		Observer() { }

		virtual ~Observer() { }

		void Respond(T *data) {
			Callback(data);
		}

		void setCallback(std::function<void(T *)> callback) {
			Callback = callback;
		}
};

template <typename T>
class Event {
	private:
		std::vector<std::shared_ptr<Observer<T>>> Observers;
	public:
		Event() {}
		virtual ~Event() {}

		void Trigger(T *data) {
			for (auto& observer : Observers) {
				if (observer) observer->Respond(data);
			}
		}

		void AddObserver(std::shared_ptr<Observer<T>> obs) {
			Observers.push_back(obs);
		}

		void RemoveObserver(std::shared_ptr<Observer<T>> obs) {
			Observers.erase(
				std::remove(Observers.begin(), Observers.end(), obs),
				Observers.end()
			);
		}
};

class ScheduledModule {
	private:

	protected:
		Event<uint32_t> CallNextModuleEvent;
		std::shared_ptr<Observer<uint32_t>> ExecTaskObserver;
		int Freq;
		uint32_t StartTime;

	public:
		ScheduledModule(int freq) {
			Freq= freq;
			ExecTaskObserver = std::make_shared<Observer<uint32_t>>();
			ExecTaskObserver->setCallback(std::bind(&ScheduledModule::ExecMainTask, this));

		}


		void SetNextModule(ScheduledModule *nextModule) {
			CallNextModuleEvent.AddObserver(nextModule->ExecTaskObserver);
		}

		virtual void ExecMainTask()= 0;

};

#endif /* INC_EVENTMANAGEMENT_H_ */
