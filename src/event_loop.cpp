#include "pwre.hpp"
#include <iostream>
#include <mutex>
#include <vector>

namespace Pwre {
	bool CheckoutNativeEvents();
	bool WaitNativeEvent();

	struct IdleHandlerInfo {
		std::function<void()> h;
	};

	std::vector<IdleHandlerInfo> ihs;
	std::mutex ihsMux;

	std::function<void()> AddIdleHandler(const std::function<void()> &h) {
		std::unique_lock<std::mutex> ul(ihsMux);
		ihs.push_back({h});
		auto it = --ihs.end();
		return [it]() {
			std::unique_lock<std::mutex> ul(ihsMux);
			ihs.erase(it);
		};
	}

	bool HandleEvent() {
		std::unique_lock<std::mutex> ul(ihsMux);
		if (ihs.size()) {
			bool ret = CheckoutNativeEvents();
			for (auto it = ihs.begin(); it != ihs.end(); it++) {
				(*it).h();
			}
			return ret;
		} else {
			return WaitNativeEvent();
		}
	}

	void WaitQuit() {
		while (HandleEvent());
	}

	void Wait(const std::atomic<bool> &flag) {
		while (!flag) {
			if (!HandleEvent()) {
				std::cout << "Pwre::Wait: GUI environment quit!" << std::endl;
				exit(1);
			}
		}
	}
} /* Pwre */
