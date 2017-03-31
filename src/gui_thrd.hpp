#include <thread>
#include <iostream>

namespace Pwre {
	class GUIThrdEntryPoint {
		public:
			const std::thread::id id;

			void Init();

			GUIThrdEntryPoint() : id(std::this_thread::get_id()) {
				Init();
			}
	};

	extern GUIThrdEntryPoint guiThrdInfo;

	#define IsGUIThrd (std::this_thread::get_id() == guiThrdInfo.id)
	#define IsNonGUIThrd (std::this_thread::get_id() != guiThrdInfo.id)

	#ifdef DEBUG
		#define AssertNonGUIThrd(n) if (std::this_thread::get_id() != guiThrdInfo.id) { std::cout << "Pwre::"#n": this is non-GUI thread!" << std::endl; exit(1); }
	#else
		#define AssertNonGUIThrd(n)
	#endif
} /* Pwre */
