#include "pwre.hpp"
#include "gui_thrd.hpp"
#include <mutex>
#include <memory>
#include <deque>

namespace Pwre {
	bool CheckoutEvents(); // In {target_plat}.cpp
	bool WaitEvent(); // In {target_plat}.cpp

	struct TaskInfo {
		std::function<void()> func;
		size_t count;
		std::shared_ptr<bool> deleted;
	};

	std::vector<TaskInfo> tasks;

	struct NonGUIThrdChangeTaskRequest {
		TaskInfo info;
		std::vector<TaskInfo>::iterator it;
	};

	std::deque<NonGUIThrdChangeTaskRequest> nonGUIThrdChangeTaskRequests;
	std::mutex nonGUIThrdChangeTaskRequestsMux;

	std::function<void()> AddTask(const std::function<void()> &func, size_t count, size_t interval) {
		if (interval) {
			std::shared_ptr<std::atomic<bool>> deleted(new std::atomic<bool>(false));

			std::thread thrd([func, interval, deleted](size_t count) {
				for (;;) {
					std::this_thread::sleep_for(std::chrono::milliseconds(interval));
					if (*deleted) {
						return;
					}
					nonGUIThrdChangeTaskRequestsMux.lock();
					nonGUIThrdChangeTaskRequests.push_back({{func, 1, nullptr}, {}});
					nonGUIThrdChangeTaskRequestsMux.unlock();
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
			std::shared_ptr<bool> deleted(new bool(false));

			if (IsGUIThrd) {
				tasks.push_back({func, count, deleted});
			} else {
				nonGUIThrdChangeTaskRequestsMux.lock();
				nonGUIThrdChangeTaskRequests.push_back({{func, count, deleted}, {}});
				nonGUIThrdChangeTaskRequestsMux.unlock();
				WakeUp();
			}

			auto it = --tasks.end();
			return [it, deleted]() {
				if (IsGUIThrd) {
					if (!*deleted) {
						tasks.erase(it);
					}
				} else {
					nonGUIThrdChangeTaskRequestsMux.lock();
					nonGUIThrdChangeTaskRequests.push_back({{std::function<void()>(nullptr), 0, deleted}, it});
					nonGUIThrdChangeTaskRequestsMux.unlock();
					WakeUp();
				}
			};
		}
	}

	void OnWakeUP() {
		AssertNonGUIThrd(OnWakeUP);

		nonGUIThrdChangeTaskRequestsMux.lock();
		for (;;) {
			const NonGUIThrdChangeTaskRequest &req = nonGUIThrdChangeTaskRequests.front();

			if (req.info.func) { // An add request.
				tasks.push_back(req.info);
			} else {  // A delete request.
				if (!*req.info.deleted) {
					tasks.erase(req.it);
				}
			}

			nonGUIThrdChangeTaskRequests.pop_front();
			if (!nonGUIThrdChangeTaskRequests.size()) {
				break;
			}
		}
		nonGUIThrdChangeTaskRequestsMux.unlock();
	}

	bool Working() {
		if (tasks.size()) {
			if (!CheckoutEvents()) {
				return false;
			}
			for (auto it = tasks.begin(); it != tasks.end(); ) {
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
