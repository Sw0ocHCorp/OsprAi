/*
 * EventManagement.h
 *
 *  Created on: Jul 24, 2025
 *      Author: nclsr
 */

#ifndef INC_EVENTMANAGEMENT_H_
#define INC_EVENTMANAGEMENT_H_

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

		virtual void Respond(T *data)= 0;

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

#endif /* INC_EVENTMANAGEMENT_H_ */
