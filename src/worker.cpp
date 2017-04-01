#include "pwre.hpp"
#include "gui_thrd.hpp"
#include <mutex>
#include <memory>

namespace Pwre {
	bool CheckoutEvents();
	bool WaitEvent();

	struct TaskInfo {
		std::function<void()> func;
		size_t count;
		std::shared_ptr<bool> deleted;
	};

	std::vector<TaskInfo> tasks;
	std::mutex tasksMux;

	std::function<void()> AddTask(const std::function<void()> &func, size_t count, size_t interval) {
		if (interval) {
			std::shared_ptr<std::atomic<bool>> deleted(new std::atomic<bool>(false));

			std::thread thrd([func, interval, deleted](size_t count) {
				for (;;) {
					std::this_thread::sleep_for(std::chrono::milliseconds(interval));
					if (*deleted) {
						return;
					}
					tasksMux.lock();
					tasks.push_back({func, 1, nullptr});
					tasksMux.unlock();
					WakeUp();
					if (count && !--count) {
						return;
					}
				}
			}, count);

			return [deleted]() {
				*deleted = true;
			};
		} else {
			std::unique_lock<std::mutex> ul(tasksMux);

			std::shared_ptr<bool> deleted(new bool(false));

			tasks.push_back({func, count, deleted});
			if (IsNonGUIThrd) {
				WakeUp();
			}

			auto it = --tasks.end();
			return [it, deleted]() {
				std::unique_lock<std::mutex> ul(tasksMux);
				if (!*deleted) {
					tasks.erase(it);
				}
			};
		}
	}

	bool Working() {
		std::unique_lock<std::mutex> ul(tasksMux);
		if (tasks.size()) {
			if (!CheckoutEvents()) {
				return false;
			}
			for (auto it = tasks.begin(); it != tasks.end();) {
				(*it).func();
				if ((*it).count && !--(*it).count) {
					if ((*it).deleted != nullptr) {
						*(*it).deleted = true;
					}
					it = tasks.erase(it);
				} else {
					++it;
				}
			}
			return true;
		} else {
			return WaitEvent();
		}
	}

	void WaitQuit() {
		AssertNonGUIThrd(WaitQuit);

		while (Working());
	}

	void Wait(const std::atomic<bool> &flag) {
		AssertNonGUIThrd(Wait);

		while (!flag) {
			if (!Working()) {
				std::cout << "Pwre::Wait: GUI environment quit!" << std::endl;
				exit(1);
			}
		}
	}
} /* Pwre */
