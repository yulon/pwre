#include <iostream>
#include <pwre.hpp>

int main() {
	Pwre::Window wnd;
	wnd.Retitle("我只是一个空白窗口_(:з」∠)_");
	wnd.Resize(500, 500);
	wnd.AddStates(PWRE_STATE_VISIBLE);
	wnd.Move();

	std::cout << "title: " << wnd.Title() << std::endl;

	Pwre::WaitQuit();
	return 0;
}
