#ifndef INC_EVENTMANAGEMENT_H_
#define INC_EVENTMANAGEMENT_H_

#include <functional>
#include <memory>
#include "Utils.hpp"

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

		void respond(T *data) {
			Callback(data);
		}

		void setCallback(std::function<void(T *)> callback) {
			Callback = callback;
		}
};

template <typename T>
class Event {
	private:
		StaticVector<std::shared_ptr<Observer<T>>, 10> Observers;
	public:
		Event() {}
		virtual ~Event() {}

		void trigger(T *data) {
            for (int i= 0; i < Observers.size(); i++) {
                Observers[i]->respond(data);
            }
		}

		void addObserver(std::shared_ptr<Observer<T>> obs) {
			Observers.add(obs);
		}

		void removeObserver(std::shared_ptr<Observer<T>> obs) {
            for (int i= 0; i < Observers.size(); i++) {
                if (Observers[i] == obs)
                    Observers.removeAt(i);
            }
		}
};

class ScheduledModule {
	private:
		bool IsFirst= false;
	protected:
		Event<void> CallNextModuleEvent;
		std::shared_ptr<Observer<void>> ExecTaskObserver;
		int Freq;
		uint32_t StartTime;
		bool IsMultiTask;
		bool IsSecondTask= true;

	public:
		ScheduledModule(int freq, bool isMultiTask) {
			Freq= freq;
			ExecTaskObserver = std::make_shared<Observer<void>>();
			ExecTaskObserver->setCallback(std::bind(&ScheduledModule::startMainTask, this));
			IsMultiTask= isMultiTask;
		}

		void setFirstInSchedule() {
			IsFirst= true;
		}

		void callNextModule() {
			this->CallNextModuleEvent.trigger(nullptr);
		}

		void setNextModule(ScheduledModule *nextModule) {
			CallNextModuleEvent.addObserver(nextModule->ExecTaskObserver);
		}

		void startMainTask() {
			/*if (IsMultiTask) {
				if (IsFirst || (int)(HAL_GetTick() - StartTime) >= (1000 / (Freq*2)) - 1) {
					StartTime = HAL_GetTick();
					IsSecondTask= !IsSecondTask;
					if (IsSecondTask) {
						ExecSecondTask();
					} else {
						ExecMainTask();
					}
				}
			} else {
				if (IsFirst || (int)(HAL_GetTick() - StartTime) >= (1000 / Freq) - 1) {
					StartTime = HAL_GetTick();
					ExecMainTask();
				}
			}*/
		}

		virtual void execMainTask()= 0;

		virtual void execSecondTask() {

		}

};

#endif /* INC_EVENTMANAGEMENT_H_ */