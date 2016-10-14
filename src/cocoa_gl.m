#include "plat.h"

#ifdef PWRE_COCOA

#include "cocoa.h"

typedef struct PrWnd_GL {
	struct PrWnd wnd;
	NSOpenGLContext *ctx;
} *PrWnd_GL;

static void _PrWnd_GL_free(PrWnd wnd) {
	if ([NSOpenGLContext currentContext] == ((PrWnd_GL)wnd)->ctx) {
		[NSOpenGLContext clearCurrentContext];
	}
	[((PrWnd_GL)wnd)->ctx release];
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

PrWnd new_PrWnd_with_GL(int x, int y, int width, int height, int flags) {
	NSOpenGLPixelFormatAttribute glAttr;

	if (flags & PWRE_GL_V3 == PWRE_GL_V3) {
		glAttr = gl3;
	} else {
		glAttr = gl2;
	}

	PrWnd_GL glWnd = (PrWnd_GL)_alloc_PrWnd(sizeof(struct PrWnd_GL), x, y, width, height);
	glWnd->wnd.onFree = _PrWnd_GL_free;

	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttr];
	glWnd->ctx = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
	[glWnd->ctx setView:[((PrWnd)glWnd)->nsWnd contentView]];
	[pixelFormat release];

	return (PrWnd)glWnd;
}

void PrWnd_GL_makeCurrent(PrWnd wnd) {
	[((PrWnd_GL)wnd)->ctx makeCurrentContext];
}

void PrWnd_GL_swapBuffers(PrWnd wnd) {
	[((PrWnd_GL)wnd)->ctx flushBuffer];
}

#endif // PWRE_COCOA
