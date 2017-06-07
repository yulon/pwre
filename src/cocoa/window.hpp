#include <pwre.hpp>
#import <Cocoa/Cocoa.h>

namespace pwre {
	class _window;
} /* pwre */

@interface PwreController : NSWindowController {
	@public
		pwre::_window *wnd;
}
- (void)mouseDown:(NSEvent *)e;
@end

@interface PwreObserver : NSObject {
	@public
		pwre::_window *wnd;
}
- (void)onSize:(NSNotification *)n;
@end

namespace pwre {
	class _window : public window {
		public:
			NSWindow *ns;
			PwreController *ctrlr;
			PwreObserver *obsrv;

			////////////////////////////////////////////////////////////////////

			_window(uint64_t hints);

			virtual void close();
			virtual void destroy();

			virtual uintptr_t native_handle();

			virtual std::string title();
			virtual void retitle(const std::string &);

			virtual point pos();
			virtual void move(point);

			virtual pwre::size size();
			virtual void resize(pwre::size);

			virtual void add_states(uint32_t);
			virtual void rm_states(uint32_t);
			virtual bool has_states(uint32_t);

			virtual void less(bool);
			//virtual bool rearea(const action_area &);
	};
} /* pwre */
