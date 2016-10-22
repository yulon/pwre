#include "plat.h"

#ifdef PWRE_PLAT_COCOA

#include "cocoa.h"

typedef struct gl_wnd {
	struct pwre_wnd wnd;
	NSOpenGLContext *ctx;
} *gl_wnd_t;

static void gl_free(pwre_wnd_t wnd) {
	if ([NSOpenGLContext currentContext] == ((gl_wnd_t)wnd)->ctx) {
		[NSOpenGLContext clearCurrentContext];
	}
	[((gl_wnd_t)wnd)->ctx release];
}

static const NSOpenGLPixelFormatAttribute gl2[] = {
	NSOpenGLPFADoubleBuffer,
	0
};

static const NSOpenGLPixelFormatAttribute gl3[] = {
	NSOpenGLPFADoubleBuffer,
	NSOpenGLPFAOpenGLProfile,
	NSOpenGLProfileVersion3_2Core,
	0
};

pwre_wnd_t pwre_new_wnd_with_gl(uint64_t hints) {
	NSOpenGLPixelFormatAttribute gl_attr;

	if (hints & PWRE_HINT_GL_V3 == PWRE_HINT_GL_V3) {
		gl_attr = gl3;
	} else {
		gl_attr = gl2;
	}

	gl_wnd_t glwnd = (gl_wnd_t)alloc_wnd(sizeof(struct gl_wnd), hints);
	glwnd->wnd.on_free = gl_free;

	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl_attr];
	glwnd->ctx = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
	[glwnd->ctx setView:[((pwre_wnd_t)glwnd)->NSWnd contentView]];
	[pixelFormat release];

	return (pwre_wnd_t)glwnd;
}

void pwre_gl_make_current(pwre_wnd_t wnd) {
	[((gl_wnd_t)wnd)->ctx makeCurrentContext];
}

void pwre_gl_swap_buffers(pwre_wnd_t wnd) {
	[((gl_wnd_t)wnd)->ctx flushBuffer];
}

#endif // PWRE_PLAT_COCOA
