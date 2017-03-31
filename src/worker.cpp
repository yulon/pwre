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
	};

	std::vector<TaskInfo> tasks;
	std::mutex tasksMux;

	std::function<void()> AddTask(const std::function<void()> &func, size_t count, size_t interval) {
		std::unique_lock<std::mutex> ul(tasksMux);

		if (interval) {
			std::shared_ptr<std::atomic<bool>> deleted(new std::atomic<bool>(false));

			std::thread thrd([func, interval, deleted](size_t count) {
				for (;;) {
					std::this_thread::sleep_for(std::chrono::milliseconds(interval));
					if (*deleted) {
						return;
					}
					tasksMux.lock();
					tasks.push_back({func, 1});
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
			tasks.push_back({func, count});
			if (IsNonGUIThrd) {
				WakeUp();
			}

			auto it = --tasks.end();
			return [it]() {
				std::unique_lock<std::mutex> ul(tasksMux);
				tasks.erase(it);
			};
		}
	}

	bool Work() {
		std::unique_lock<std::mutex> ul(tasksMux);
		if (tasks.size()) {
			bool ret = CheckoutEvents();
			for (auto it = tasks.begin(); it != tasks.end();) {
				(*it).func();
				if ((*it).count && !--(*it).count) {
					it = tasks.erase(it);
				} else {
					++it;
				}
			}
			return ret;
		} else {
			return WaitEvent();
		}
	}

	void WaitQuit() {
		AssertNonGUIThrd(WaitQuit);

		while (Work());
	}

	void Wait(const std::atomic<bool> &flag) {
		AssertNonGUIThrd(Wait);

		while (!flag) {
			if (!Work()) {
				std::cout << "Pwre::Wait: GUI environment quit!" << std::endl;
				exit(1);
			}
		}
	}
} /* Pwre */
