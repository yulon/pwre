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

PrWnd new_PrWnd_with_GL(uint64_t hints) {
	NSOpenGLPixelFormatAttribute glAttr;

	if (hints & PWRE_HINT_GL_V3 == PWRE_HINT_GL_V3) {
		glAttr = gl3;
	} else {
		glAttr = gl2;
	}

	PrWnd_GL glWnd = (PrWnd_GL)_alloc_PrWnd(sizeof(struct PrWnd_GL), hints);
	glWnd->wnd.onFree = _PrWnd_GL_free;

	NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttr];
	glWnd->ctx = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
	[glWnd->ctx setView:[((PrWnd)glWnd)->nsWnd contentView]];
	[pixelFormat release];

	return (PrWnd)glWnd;
}

void PrWnd_GL_MakeCurrent(PrWnd wnd) {
	[((PrWnd_GL)wnd)->ctx makeCurrentContext];
}

void PrWnd_GL_SwapBuffers(PrWnd wnd) {
	[((PrWnd_GL)wnd)->ctx flushBuffer];
}

#endif // PWRE_COCOA
