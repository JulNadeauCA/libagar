/*
 * Copyright (c) 2012-2020 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Driver for Cocoa framework. This is a multiple display driver (one
 * Cocoa window is created for each Agar window).
 */

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/text.h>
#include <agar/gui/cursors.h>

#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/gl.h>

#include <agar/gui/drv_gl_common.h>
#include <agar/gui/drv_cocoa_keymap.h>

static int nDrivers = 0;		/* Drivers open */
static AG_DriverEventQ cocEventQ;	/* Private event queue */
static AG_EventSink   *cocEventSpinner = NULL;  /* Standard event sink */
static AG_EventSink   *cocEventEpilogue = NULL; /* Standard event epilogue */
struct ag_driver_cocoa;

/*
 * Application delegate
 */

@interface AG_AppDelegate : NSObject
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
@end

@implementation AG_AppDelegate : NSObject
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
{
	AG_QuitGUI();
	return (NSTerminateCancel);
}
@end

/*
 * Window interface
 */
@interface AG_CocoaWindow : NSWindow {
@public
	AG_Window *_agarWindow;		/* Corresponding Agar window */
}

- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;
@end

@implementation AG_CocoaWindow
- (BOOL)canBecomeKeyWindow
{
	return (_agarWindow->flags & AG_WINDOW_DENYFOCUS) ? NO : YES;
}

- (BOOL)canBecomeMainWindow
{
	return (_agarWindow->flags & AG_WINDOW_MAIN) ? YES : NO;
}
@end

/*
 * View interface
 */
@interface AG_CocoaView : NSView {
@public
	AG_Window *_agarWindow;		/* Corresponding Agar window */
}
- (void)rightMouseDown:(NSEvent *)theEvent;
- (BOOL)preservesContentDuringLiveResize;
- (void)viewWillStartLiveResize;
- (void)viewDidEndLiveResize;
@end

@implementation AG_CocoaView
- (void)rightMouseDown:(NSEvent *)theEvent
{
	[[self nextResponder] rightMouseDown:theEvent];
}

- (BOOL)preservesContentDuringLiveResize
{
	return (YES);
}

- (void)viewWillStartLiveResize
{
	[super viewWillStartLiveResize];
}

- (void)viewDidEndLiveResize
{
	[super viewDidEndLiveResize];
}
@end

/*
 * Event listener interface
 */
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
@interface AG_CocoaListener : NSResponder <NSWindowDelegate> {
#else
@interface AG_CocoaListener : NSResponder {
#endif
	AG_Window *_window;
	struct ag_driver_cocoa *_driver;
}

-(void) listen:(struct ag_driver_cocoa *) driver;
-(void) close;

/* Window delegate functionality */
-(BOOL) windowShouldClose:(id) sender;
-(void) windowDidExpose:(NSNotification *) aNotification;
-(void) windowDidMove:(NSNotification *) aNotification;
-(void) windowDidResize:(NSNotification *) aNotification;
-(void) windowDidMiniaturize:(NSNotification *) aNotification;
-(void) windowDidDeminiaturize:(NSNotification *) aNotification;
-(void) windowDidBecomeKey:(NSNotification *) aNotification;
-(void) windowDidResignKey:(NSNotification *) aNotification;

@end

/* Driver instance data */
typedef struct ag_driver_cocoa {
	struct ag_driver_mw _inherit;
	AG_CocoaWindow   *win;		/* Cocoa window */
	AG_CocoaListener *evListener;	/* Cocoa event listener */
	NSOpenGLContext *glCtx;
	AG_GL_Context    gl;
	AG_Mutex         lock;		/* Protect Cocoa calls */
	Uint             modFlags;	/* Last modifier state */
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
	NSTrackingArea  *trackArea;	/* For mouse motion events */
#endif
} AG_DriverCocoa;

AG_DriverMwClass agDriverCocoa;

#define AGDRIVER_IS_COCOA(drv) \
	(AGDRIVER_CLASS(drv) == (AG_DriverClass *)&agDriverCocoa)


static int  COCOA_PendingEvents(void *_Nonnull);
static int  COCOA_GetNextEvent(void *_Nonnull, AG_DriverEvent *_Nonnull);
static int  COCOA_ProcessEvent(void *_Nonnull, AG_DriverEvent *_Nonnull);
static void COCOA_PostResizeCallback(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);
static void COCOA_PostMoveCallback(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);
static int  COCOA_RaiseWindow(AG_Window *_Nonnull);
static int  COCOA_SetInputFocus(AG_Window *_Nonnull);
#if 0
static void COCOA_FreeWidgetResources(AG_Widget *_Nonnull);
#endif

static __inline__ void
ConvertNSRect(NSRect *_Nonnull r)
{
	r->origin.y = CGDisplayPixelsHigh(kCGDirectMainDisplay) - 
	              r->origin.y - r->size.height;
}

@implementation AG_CocoaListener

- (void) listen:(AG_DriverCocoa *)co
{	
	NSNotificationCenter *nc;
	NSWindow *win = co->win;
	NSView *view = [win contentView];

	_driver = co;
	_window = AGDRIVER_MW(co)->win;

	nc = [NSNotificationCenter defaultCenter];

	if ([win delegate] != nil) {
		[nc addObserver:self selector:@selector(windowDidExpose:) name:NSWindowDidExposeNotification object:win];
		[nc addObserver:self selector:@selector(windowDidMove:) name:NSWindowDidMoveNotification object:win];
		[nc addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:win];
		[nc addObserver:self selector:@selector(windowDidMiniaturize:) name:NSWindowDidMiniaturizeNotification object:win];
		[nc addObserver:self selector:@selector(windowDidDeminiaturize:) name:NSWindowDidDeminiaturizeNotification object:win];
		[nc addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:win];
		[nc addObserver:self selector:@selector(windowDidResignKey:) name:NSWindowDidResignKeyNotification object:win];
	} else {
		[win setDelegate:self];
	}

	[win setNextResponder:self];
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1050
	[win setAcceptsMouseMovedEvents:YES];
#endif
	[view setNextResponder:self];

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
	if ([view respondsToSelector:@selector(setAcceptsTouchEvents:)])
		[view setAcceptsTouchEvents:YES];
#endif
}

- (void)close
{
	NSNotificationCenter *nc;
	AG_DriverCocoa *co = _driver;
	NSWindow *win = co->win;
	NSView *view = [win contentView];
	
	nc = [NSNotificationCenter defaultCenter];

	if ([win delegate] != self) {
		[nc removeObserver:self name:NSWindowDidExposeNotification object:win];
		[nc removeObserver:self name:NSWindowDidMoveNotification object:win];
		[nc removeObserver:self name:NSWindowDidResizeNotification object:win];
		[nc removeObserver:self name:NSWindowDidMiniaturizeNotification object:win];
		[nc removeObserver:self name:NSWindowDidDeminiaturizeNotification object:win];
		[nc removeObserver:self name:NSWindowDidBecomeKeyNotification object:win];
		[nc removeObserver:self name:NSWindowDidResignKeyNotification object:win];
	} else {
		[win setDelegate:nil];
	}

	if ([win nextResponder] == self) {
		[win setNextResponder:nil];
	}
	if ([view nextResponder] == self) {
		[view setNextResponder:nil];
	}
}

- (BOOL)windowShouldClose:(id)sender
{
	AG_DriverEvent *dev;

	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) != NULL) {
		dev->type = AG_DRIVER_CLOSE;
		dev->win = _window;
		TAILQ_INSERT_TAIL(&cocEventQ, dev, events);
	}
	return NO;
}

- (void)windowDidExpose:(NSNotification *)aNotification
{
	AG_DriverEvent *dev;

	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) != NULL) {
		dev->type = AG_DRIVER_EXPOSE;
		dev->win = _window;
		TAILQ_INSERT_TAIL(&cocEventQ, dev, events);
	}
}

- (void)windowDidMove:(NSNotification *)aNotification
{
	AG_DriverCocoa *co = _driver;
	AG_Window *win = _window;
	NSRect rect;
	AG_SizeAlloc a;

	rect = [co->win contentRectForFrameRect:[co->win frame]];
	ConvertNSRect(&rect);

	a.x = (int)rect.origin.x;
	a.y = (int)rect.origin.y;
	a.w = WIDGET(win)->w;
	a.h = WIDGET(win)->h;
	COCOA_PostMoveCallback(win, &a);
}

- (void)windowDidResize:(NSNotification *)aNotification
{
	AG_Window *win = _window;
	AG_DriverCocoa *co = _driver;
	AG_SizeAlloc a;
	NSRect rect;

	rect = [co->win contentRectForFrameRect:[co->win frame]];
	ConvertNSRect(&rect);

	/*
	 * Since the event loop will not run during a live resize
	 * operation, we can't use AG_DRIVER_VIDEORESIZE, and we
	 * must redraw the window immediately.
	 */
	a.x = (int)rect.origin.x;
	a.y = (int)rect.origin.y;
	a.w = (int)rect.size.width;
	a.h = (int)rect.size.height;

	AG_MutexLock(&co->lock);
	AG_ObjectLock(win);

	if (a.w != WIDTH(win) || a.h != HEIGHT(win)) {
		COCOA_PostResizeCallback(win, &a);
	} else {
		COCOA_PostMoveCallback(win, &a);
	}
	if (win->visible) {
		AG_BeginRendering(_driver);
		AG_WindowDraw(win);
		AG_EndRendering(_driver);
	}

	AG_ObjectUnlock(win);
	AG_MutexUnlock(&co->lock);
}

- (void)windowDidMiniaturize:(NSNotification *)aNotification
{
	_window->flags |= AG_WINDOW_MINIMIZED;
}

- (void)windowDidDeminiaturize:(NSNotification *)aNotification
{
	_window->flags &= ~(AG_WINDOW_MINIMIZED);
}

- (void)windowDidBecomeKey:(NSNotification *)aNotification
{
	AG_DriverEvent *dev;

	agWindowFocused = _window;

	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) != NULL) {
		dev->type = AG_DRIVER_FOCUS_IN;
		dev->win = _window;
		TAILQ_INSERT_TAIL(&cocEventQ, dev, events);
	}
}

- (void)windowDidResignKey:(NSNotification *)aNotification
{
	AG_DriverEvent *dev;

	if (agWindowFocused != _window) {
		return;
	}
	agWindowFocused = NULL;
	
	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) != NULL) {
		dev->type = AG_DRIVER_FOCUS_OUT;
		dev->win = _window;
		TAILQ_INSERT_TAIL(&cocEventQ, dev, events);
	}
}

@end

static void
Init(void *_Nonnull obj)
{
	AG_DriverCocoa *co = obj;

	co->win = NULL;
	co->glCtx = NULL;
	co->modFlags = 0;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
	co->trackArea = nil;
#endif
	AG_MutexInitRecursive(&co->lock);
}

static void
Destroy(void *_Nonnull obj)
{
	AG_DriverCocoa *co = obj;

	AG_MutexDestroy(&co->lock);
}

/*
 * Standard AG_EventLoop() event sink.
 */
static int
COCOA_EventSink(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	AG_DriverEvent dev;

	if (COCOA_GetNextEvent(NULL, &dev) == 1) {
		return COCOA_ProcessEvent(NULL, &dev);
	}
	return (0);
}
static int
COCOA_EventEpilogue(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	AG_WindowDrawQueued();
	AG_WindowProcessQueued();
	return (0);
}

static int
COCOA_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverCocoa *co = obj;
	NSAutoreleasePool *pool;
	
	/* Driver manages rendering of window background. */
	drv->flags |= AG_DRIVER_WINDOW_BG;

	/* Initialize NSApp if needed. */
	pool = [[NSAutoreleasePool alloc] init];
	if (NSApp == nil) {
		NSApp = [NSApplication sharedApplication];
		[NSApp finishLaunching];
	}
	if ([NSApp delegate] == nil) {
		[NSApp setDelegate:[[AG_AppDelegate alloc] init]];
	}
	[pool release];
	
	/* Initialize the core mouse and keyboard */
	/* TODO: touch handling */
	if ((drv->mouse = AG_MouseNew(co, "X mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(co, "X keyboard")) == NULL) {
		goto fail;
	}
	if (nDrivers == 0) {
		TAILQ_INIT(&cocEventQ);
		
		/* Set up event filters for standard AG_EventLoop(). */
		if ((cocEventSpinner = AG_AddEventSpinner(COCOA_EventSink, NULL)) == NULL ||
		    (cocEventEpilogue = AG_AddEventEpilogue(COCOA_EventEpilogue, NULL)) == NULL)
			goto fail;
	}
	nDrivers++;
	return (0);
fail:
	if (cocEventSpinner != NULL) { AG_DelEventSpinner(cocEventSpinner); cocEventSpinner = NULL; }
	if (cocEventEpilogue != NULL) { AG_DelEventEpilogue(cocEventEpilogue); cocEventEpilogue = NULL; }
	if (drv->kbd != NULL) { AG_ObjectDelete(drv->kbd); drv->kbd = NULL; }
	if (drv->mouse != NULL) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	return (-1);
}

static void
COCOA_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	if (--nDrivers == 0) {
		AG_DriverEvent *dev, *devNext;
		
		AG_DelEventSink(cocEventSpinner); cocEventSpinner = NULL;
		AG_DelEventEpilogue(cocEventEpilogue); cocEventEpilogue = NULL;

		for (dev = TAILQ_FIRST(&cocEventQ);
		     dev != TAILQ_LAST(&cocEventQ, ag_driver_eventq);
		     dev = devNext) {
			devNext = TAILQ_NEXT(dev, events);
			Free(dev);
		}
		TAILQ_INIT(&cocEventQ);
	}

	AG_ObjectDelete(drv->mouse); drv->mouse = NULL;
	AG_ObjectDelete(drv->kbd); drv->kbd = NULL;
}

static int
COCOA_GetDisplaySize(Uint *_Nonnull w, Uint *_Nonnull h)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSScreen *screen;
	NSRect sr;

	/* XXX this is probably wrong */
	screen = [NSScreen mainScreen];
	sr = [screen frame];
	*w = sr.size.width;
	*h = sr.size.height;
	[pool release];
	return (0);
}

static int
COCOA_PendingEvents(void *_Nonnull drvCaller)
{
	NSAutoreleasePool *pool;
	NSEvent *ev;
	int rv;

	if (!TAILQ_EMPTY(&cocEventQ))
		return (1);

	pool = [[NSAutoreleasePool alloc] init];
	ev = [NSApp nextEventMatchingMask:NSAnyEventMask
	                                  untilDate:[NSDate distantPast]
					  inMode:NSDefaultRunLoopMode
					  dequeue:NO];
	rv = (ev != nil);

	[pool release];
	return (rv);
}

/* Convert a NSEvent mouse button number to AG_MouseButton. */
static AG_MouseButton _Const_Attribute
GetMouseButton(int which)
{
	switch (which) {
	case 0:		return (AG_MOUSE_LEFT);
	case 1:		return (AG_MOUSE_RIGHT);
	case 2:		return (AG_MOUSE_MIDDLE);
	default:	return (which+1);
	}
}

static AG_MouseButton _Const_Attribute
GetScrollWheelButton(float x, float y)
{
	if (x > 0) {
		return (AG_MOUSE_X1);
	} else if (x < 0) {
		return (AG_MOUSE_X2);
	}
	if (y > 0) {
		return (AG_MOUSE_WHEELUP);
	} else if (y < 0) {
		return (AG_MOUSE_WHEELDOWN);
	}
	return (AG_MOUSE_NONE);
}

/* Add a keyboard event to the queue. */
static void
QueueKeyEvent(AG_DriverCocoa *_Nonnull co, enum ag_driver_event_type type,
    AG_KeySym ks, Uint32 ucs)
{
	AG_DriverEvent *dev;

	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) == NULL) {
		AG_Verbose("Out of memory for keymod event\n");
		return;
	}
	dev->type = type;
	dev->win = AGDRIVER_MW(co)->win;
	dev->data.key.ks = ks;
	dev->data.key.ucs = ucs;
	TAILQ_INSERT_TAIL(&cocEventQ, dev, events);
}

static int
COCOA_GetNextEvent(void *_Nullable drvCaller, AG_DriverEvent *_Nonnull dev)
{
	NSAutoreleasePool *pool;
	AG_CocoaWindow *coWin;
	AG_Window *win;
	AG_Driver *drv;
	AG_DriverCocoa *co;
	AG_DriverEvent *devFirst;
	NSEvent *event;
	int rv = 0;
	
	if (!TAILQ_EMPTY(&cocEventQ))
		goto out_dequeue;

	pool = [[NSAutoreleasePool alloc] init];
	event = [NSApp nextEventMatchingMask:NSAnyEventMask
	         untilDate:[NSDate distantPast]
	         inMode:NSDefaultRunLoopMode
	         dequeue:YES];

	if (event == nil) {
		goto out;
	}

	if ((coWin = (AG_CocoaWindow *)[event window]) == nil) {
		[NSApp sendEvent:event];
		goto out;
	}
	win = coWin->_agarWindow;

	AG_LockVFS(&agDrivers);
	drv = WIDGET(win)->drv;
	co = (AG_DriverCocoa *)drv;
	
	switch ([event type]) {
	case NSMouseMoved:
	case NSLeftMouseDragged:
	case NSRightMouseDragged:
	case NSOtherMouseDragged:
		{
			NSPoint point = [event locationInWindow];
			int x = (int)point.x;
			int y = (int)(WIDGET(win)->h - point.y);

			AG_MouseMotionUpdate(drv->mouse, x, y);
			dev->type = AG_DRIVER_MOUSE_MOTION;
			dev->win = win;
			dev->data.motion.x = x;
			dev->data.motion.y = y;
			rv = 1;
			break;
		}
	case NSScrollWheel:
		{
			float x = [event deltaX];
			float y = [event deltaY];
			AG_MouseButton btn = GetScrollWheelButton(x, y);
		
			if (btn == AG_MOUSE_NONE) {
				break;
			}
			AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED, btn);
			dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
			dev->win = win;
			dev->data.button.which = btn;
			dev->data.button.x = drv->mouse->x;
			dev->data.button.y = drv->mouse->y;
			rv = 1;
			break;
		}
	case NSLeftMouseDown:
	case NSOtherMouseDown:
	case NSRightMouseDown:
		{
			AG_MouseButton btn = GetMouseButton([event buttonNumber]);
			
			AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED, btn);
			dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
			dev->win = win;
			dev->data.button.which = btn;
			dev->data.button.x = drv->mouse->x;
			dev->data.button.y = drv->mouse->y;
			rv = 1;
			break;
		}
	case NSLeftMouseUp:
	case NSOtherMouseUp:
	case NSRightMouseUp:
		{
			AG_MouseButton btn = GetMouseButton([event buttonNumber]);
			
			AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_RELEASED, btn);
			dev->type = AG_DRIVER_MOUSE_BUTTON_UP;
			dev->win = win;
			dev->data.button.which = btn;
			dev->data.button.x = drv->mouse->x;
			dev->data.button.y = drv->mouse->y;
			rv = 1;
			break;
		}
	case NSMouseEntered:
		if ([co->win isKeyWindow]) {
			dev->type = AG_DRIVER_MOUSE_ENTER;
			dev->win = win;
			rv = 1;
		} else {
			rv = 0;
		}
		break;
	case NSMouseExited:
		if ([co->win isKeyWindow]) {
			dev->type = AG_DRIVER_MOUSE_LEAVE;
			dev->win = win;
			rv = 1;
		} else {
			rv = 0;
		}
		break;
	case NSKeyDown:
		if ([event isARepeat]) {
			/* Agar implements its own key repeat */
			goto out;
		}
		/* FALLTHROUGH */
	case NSKeyUp:
		{
			NSString *characters = [event characters];
			enum ag_driver_event_type evType;
			enum ag_keyboard_action kbdAction;
			AG_KeySym ks;
			unichar c;
			NSUInteger i;

			if ([characters length] == 0)
				goto out;

			if ([event type] == NSKeyDown) {
				evType = AG_DRIVER_KEY_DOWN;
				kbdAction = AG_KEY_PRESSED;
			} else {
				evType = AG_DRIVER_KEY_UP;
				kbdAction = AG_KEY_RELEASED;
			}
	
			/* Look for matching function keys first. */
			c = [characters characterAtIndex: 0];
			ks = AG_KEY_NONE;
			for (i = 0; i < agCocoaFunctionKeysSize; i++) {
				const struct ag_cocoa_function_key *fnKey =
				    &agCocoaFunctionKeys[i];

				if (fnKey->uc == c) {
					ks = fnKey->keySym;
					break;
				}
			}
			if (ks != AG_KEY_NONE) {
				AG_KeyboardUpdate(drv->kbd, kbdAction, ks);
				dev->type = evType;
				dev->win = win;
				dev->data.key.ks = ks;
				if (ks == AG_KEY_RETURN) {
					dev->data.key.ucs = '\n';
				} else {
					dev->data.key.ucs = 0;
				}
				rv = 1;
				goto out;
			}

			/* Process as a character sequence. */
			for (i = 0; i < [characters length]; i++) {
				AG_KeySym ks;

				c = [characters characterAtIndex: i];
				ks = (c <= AG_KEY_ASCII_END) ?
				    (AG_KeySym)c : AG_KEY_NONE;
				AG_KeyboardUpdate(drv->kbd, kbdAction, ks);

				if (i == 0) {
					dev->type = evType;
					dev->win = win;
					dev->data.key.ks = ks;
					dev->data.key.ucs = (Uint32)c;
					rv = 1;
				} else {
					QueueKeyEvent(co, evType, ks,
					    (Uint32)c);
				}
			}
			if (rv == 1)
				goto out;
		}
	case NSFlagsChanged:
		{
			Uint modFlags = [event modifierFlags];
			int i, nChanged = 0;

			for (i = 0; i < agCocoaKeymodSize; i++) {
				const struct ag_cocoa_keymod_entry *kmEnt =
				    &agCocoaKeymod[i];
			
				if ((modFlags & kmEnt->keyMask) &&
				     !(co->modFlags & kmEnt->keyMask)) {
					AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED, kmEnt->keySym);
					QueueKeyEvent(co, AG_DRIVER_KEY_DOWN, kmEnt->keySym, 0);
					nChanged++;
				} else if (!(modFlags & kmEnt->keyMask) &&
				    (co->modFlags & kmEnt->keyMask)) {
					AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED, kmEnt->keySym);
					QueueKeyEvent(co, AG_DRIVER_KEY_UP, kmEnt->keySym, 0);
					nChanged++;
				}
			}
			co->modFlags = modFlags;

			if (nChanged > 0) {
				goto out_dequeue;
			}
			break;
		}
	default:
		break;
	}
	[NSApp sendEvent:event];
out:
	[pool release];
	return (rv);
out_dequeue:
	devFirst = TAILQ_FIRST(&cocEventQ);
	TAILQ_REMOVE(&cocEventQ, devFirst, events);
	memcpy(dev, devFirst, sizeof(AG_DriverEvent));
	Free(devFirst);
	return (1);
}

static int
COCOA_ProcessEvent(void *_Nullable drvCaller, AG_DriverEvent *_Nonnull dev)
{
	AG_Driver *drv;
	AG_Window *win;
	int rv=1, useText;

	if ((win = dev->win) == NULL ||
	    win->flags & AG_WINDOW_DETACHING)
		return (0);
	
	AG_LockVFS(&agDrivers);
	drv = WIDGET(win)->drv;

	if ((useText = (win->flags & AG_WINDOW_USE_TEXT))) {
		AG_PushTextState();
		AG_TextFont(WIDGET(win)->font);
		AG_TextColor(&WIDGET(win)->pal.c[WIDGET(win)->state]
		                                [AG_TEXT_COLOR]);
	}
	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		AG_ProcessMouseMotion(win,
		    dev->data.motion.x, dev->data.motion.y,
		    drv->mouse->xRel, drv->mouse->yRel,
		    drv->mouse->btnState);
		AG_MouseCursorUpdate(win,
		     dev->data.motion.x, dev->data.motion.y);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		AG_ProcessMouseButtonDown(win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		AG_ProcessMouseButtonUp(win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_KEY_UP:
		AG_ProcessKey(drv->kbd, win, AG_KEY_RELEASED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_KEY_DOWN:
		AG_ProcessKey(drv->kbd, win, AG_KEY_PRESSED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_MOUSE_ENTER:
		AG_PostEvent(win, "window-enter", NULL);
		break;
	case AG_DRIVER_MOUSE_LEAVE:
		AG_PostEvent(win, "window-leave", NULL);
		break;
	case AG_DRIVER_FOCUS_IN:
		agWindowFocused = win;
		AG_PostEvent(win, "window-gainfocus", NULL);
		break;
	case AG_DRIVER_FOCUS_OUT:
		AG_PostEvent(win, "window-lostfocus", NULL);
		break;
	case AG_DRIVER_CLOSE:
		AG_PostEvent(win, "window-close", NULL);
		break;
	case AG_DRIVER_EXPOSE:
		win->dirty = 1;
		break;
	default:
		rv = 0;
		break;
	}
	if (useText) {
		AG_PopTextState();
	}
	AG_UnlockVFS(&agDrivers);
	return (rv);
}

/* Select the window's OpenGL context. */
static __inline__ void
COCOA_GL_MakeCurrent(AG_DriverCocoa *_Nonnull co)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	[co->glCtx setView:[co->win contentView]];
	[co->glCtx update];
	[co->glCtx makeCurrentContext];

	[pool release];
}

static void
COCOA_BeginRendering(void *_Nonnull obj)
{
	AG_DriverCocoa *co = obj;

	AG_MutexLock(&co->lock);
	COCOA_GL_MakeCurrent(co);
}

static void
COCOA_RenderWindow(AG_Window *_Nonnull win)
{
	const AG_Color *cBg = &WCOLOR(win, BG_COLOR);

	AG_PushClipRect(win, &WIDGET(win)->r);
	
	glClearColor((float)cBg->r / AG_COLOR_LASTF,
	             (float)cBg->g / AG_COLOR_LASTF,
		     (float)cBg->b / AG_COLOR_LASTF, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	AG_WidgetDraw(win);

	AG_PopClipRect(win);
}

static void
COCOA_EndRendering(void *_Nonnull obj)
{
	AG_DriverCocoa *co = obj;
	AG_GL_Context *gl = &co->gl;
	Uint i;
	
	[co->glCtx flushBuffer];

	/* Remove textures and display lists queued for deletion. */
	glDeleteTextures(gl->nTextureGC, (const GLuint *)gl->textureGC);
	for (i = 0; i < gl->nListGC; i++) {
		glDeleteLists(gl->listGC[i], 1);
	}
	gl->nTextureGC = 0;
	gl->nListGC = 0;
	AG_MutexUnlock(&co->lock);
}

/*
 * Window operations
 */

static void
SetBackgroundColor(AG_DriverCocoa *_Nonnull co, const AG_Color *_Nonnull c)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	CGFloat r = (CGFloat)(c->r / AG_COLOR_LASTF);
	CGFloat g = (CGFloat)(c->g / AG_COLOR_LASTF);
	CGFloat b = (CGFloat)(c->b / AG_COLOR_LASTF);
	CGFloat a = (CGFloat)(c->a / AG_COLOR_LASTF);
	NSColor *bgColor;

	bgColor = [NSColor colorWithCalibratedRed:r green:g blue:b alpha:a];
	[co->win setBackgroundColor:bgColor];

	[pool release];
}

static int
COCOA_OpenWindow(AG_Window *_Nonnull win, const AG_Rect *_Nonnull r,
    int depthReq, Uint mwFlags)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv;
	Uint winStyle = 0;
	NSRect winRect, scrRect;
	NSArray *screens;
	NSScreen *screen, *selScreen = nil;
	AG_CocoaView *contentView;
	NSOpenGLPixelFormatAttribute pfAttr[32];
	NSOpenGLPixelFormat *pf;
	int i, count;
	AG_SizeAlloc a;

	if (depthReq == 0)
		depthReq = 24;

	AG_MutexLock(&co->lock);

	/* Set the window style. */
	if (win->flags & AG_WINDOW_NOBORDERS) {
		winStyle = NSBorderlessWindowMask;
	} else {
		if (!(win->flags & AG_WINDOW_NOTITLE)) { winStyle |= NSTitledWindowMask; }
		if (!(win->flags & AG_WINDOW_NOCLOSE)) { winStyle |= NSClosableWindowMask; }
		if (!(win->flags & AG_WINDOW_NOMINIMIZE)) { winStyle |= NSMiniaturizableWindowMask; }
		if (!(win->flags & AG_WINDOW_NORESIZE)) { winStyle |= NSResizableWindowMask; }
	}

	/* Set the window coordinates. */
	winRect.origin.x    = r->x;
	winRect.origin.y    = r->y;
	winRect.size.width  = r->w;
	winRect.size.height = r->h;
	ConvertNSRect(&winRect);

	/* Select a screen. */
	screens = [NSScreen screens];
	count = [screens count];
	for (i = 0; i < count; i++) {
		screen = [screens objectAtIndex:i];
		scrRect = [screen frame];

		if (winRect.origin.x >= scrRect.origin.x &&
		    winRect.origin.y >= scrRect.origin.y &&
		    winRect.origin.x < scrRect.origin.x + scrRect.size.width &&
		    winRect.origin.y < scrRect.origin.y + scrRect.size.height) {
		    	selScreen = screen;
			winRect.origin.x -= scrRect.origin.x;
			winRect.origin.y -= scrRect.origin.y;
			break;
		}
	}
	
	/* Create the window. */
	if (selScreen == nil) {
		co->win = [[AG_CocoaWindow alloc] initWithContentRect:winRect
		                                  styleMask:winStyle
					          backing:NSBackingStoreBuffered
					          defer:YES];
	} else {
		co->win = [[AG_CocoaWindow alloc] initWithContentRect:winRect
		                                  styleMask:winStyle
					          backing:NSBackingStoreBuffered
					          defer:YES
					          screen:selScreen];
	}
	co->win->_agarWindow = win;
	SetBackgroundColor(co, &WCOLOR(win, BG_COLOR));

	if (win->flags & AG_WINDOW_MAIN)
		[co->win makeMainWindow];

	/* Create an event listener. */
	co->evListener = [[AG_CocoaListener alloc] init];

	/* Obtain the effective window coordinates; set up our contentView. */
	winRect = [co->win contentRectForFrameRect:[co->win frame]];
	contentView = [[AG_CocoaView alloc] initWithFrame:winRect];
	contentView->_agarWindow = win;
	[co->win setContentView: contentView];
	[contentView release];
	
	/* Set our event listener. */
	[co->evListener listen:co];

	/* Retrieve the effective style flags. */
	winStyle = [co->win styleMask];
	if (winStyle == NSBorderlessWindowMask) {
		win->flags |= AG_WINDOW_NOBORDERS;
	} else {
		if (!(winStyle & NSTitledWindowMask)) { win->flags |= AG_WINDOW_NOTITLE; }
		if (!(winStyle & NSClosableWindowMask)) { win->flags |= AG_WINDOW_NOCLOSE; }
		if (!(winStyle & NSMiniaturizableWindowMask)) { win->flags |= AG_WINDOW_NOMINIMIZE; }
		if (!(winStyle & NSResizableWindowMask)) { win->flags |= AG_WINDOW_NORESIZE; }
	}

	/* Retrieve the effective maximize/minimize and focus state. */
	if (!(win->flags & AG_WINDOW_NORESIZE) && [co->win isZoomed]) {
		win->flags |= AG_WINDOW_MAXIMIZED;
	} else {
		win->flags &= ~(AG_WINDOW_MAXIMIZED);
	}
	if ([co->win isMiniaturized]) {
		win->flags |= AG_WINDOW_MINIMIZED;
	} else {
		win->flags &= ~(AG_WINDOW_MINIMIZED);
	}
	if ([co->win isKeyWindow])
		agWindowFocused = win;

	/* Create an OpenGL rendering context. */
	i = 0;
	pfAttr[i++] = NSOpenGLPFADepthSize;
	pfAttr[i++] = depthReq;
	pfAttr[i++] = NSOpenGLPFADoubleBuffer;
	if (agStereo) { pfAttr[i++] = NSOpenGLPFAStereo; }
	pfAttr[i] = 0;
	pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:pfAttr];
	if (pf == nil) {
		AG_SetError("Cannot create NSOpenGLPixelFormat");
		goto fail;
	}
	co->glCtx = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
	[pf release];
	if (co->glCtx == nil) {
		AG_SetError("Cannot create NSOpenGLContext");
		goto fail;
	}
	[co->glCtx update];
	[co->glCtx makeCurrentContext];
	AG_GL_InitContext(co, &co->gl);

	/* XXX TODO: how to check effective depth? */

	/* Set the pixel formats. */
	if ((drv->videoFmt = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		goto fail_ctx;
	}
#if AG_MODEL == AG_LARGE
	if (depthReq == 48) {				/* Deep color */
# if AG_BYTEORDER == AG_BIG_ENDIAN
		AG_PixelFormatRGB(drv->videoFmt, depthReq,
			0xffff000000000000,
			0x0000ffff00000000,
			0x00000000ffff0000);
# else
		AG_PixelFormatRGB(drv->videoFmt, depthReq,
			0x000000000000ffff,
			0x00000000ffff0000,
			0x0000ffff00000000);
# endif
	} else
#endif /* AG_LARGE */
	{						/* True Color */
#if AG_BYTEORDER == AG_BIG_ENDIAN
		AG_PixelFormatRGB(drv->videoFmt, depthReq,
			0xff000000,
			0x00ff0000,
			0x0000ff00);
#else
		AG_PixelFormatRGB(drv->videoFmt, depthReq,
			0x000000ff,
			0x0000ff00,
			0x00ff0000);
#endif
	}

	/*
	 * Set the effective window geometry, initialize the viewport
	 * and tracking rectangle.
	 */
	ConvertNSRect(&winRect);
	a.x = winRect.origin.x;
	a.y = winRect.origin.y;
	a.w = winRect.size.width;
	a.h = winRect.size.height;
	COCOA_PostResizeCallback(win, &a);
	
	AG_MutexUnlock(&co->lock);
	[pool release];
	return (0);
fail_ctx:
	AG_GL_DestroyContext(co);
fail:
	[NSOpenGLContext clearCurrentContext];
	[co->evListener close];
	[co->evListener release];
	[co->win close];
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	AG_MutexUnlock(&co->lock);
	[pool release];
	return (-1);
}

static void
COCOA_CloseWindow(AG_Window *_Nonnull win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverCocoa *co = (AG_DriverCocoa *)drv;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	AG_MutexLock(&co->lock);

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
	if (co->trackArea != nil) {
		[[co->win contentView] removeTrackingArea:co->trackArea];
		[co->trackArea release];
		co->trackArea = nil;
	}
#endif
	/* Destroy our OpenGL rendering context. */
	COCOA_GL_MakeCurrent(co);
	AG_GL_DestroyContext(drv);
	[co->glCtx clearDrawable];
	[co->glCtx release];

	/* Close the window. */
	[co->evListener close];
	[co->evListener release];
	[co->win close];

	AG_PixelFormatFree(drv->videoFmt);
	free(drv->videoFmt);
	drv->videoFmt = NULL;

	AG_MutexUnlock(&co->lock);
	[pool release];
}

static int
COCOA_MapWindow(AG_Window *_Nonnull win)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	
	AG_MutexLock(&co->lock);
	if (![co->win isMiniaturized]) {
		[co->win makeKeyAndOrderFront:nil];
	}
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static int
COCOA_UnmapWindow(AG_Window *_Nonnull win)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	
	AG_MutexLock(&co->lock);
	[co->win orderOut:nil];
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static int
COCOA_RaiseWindow(AG_Window *_Nonnull win)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	
	AG_MutexLock(&co->lock);
	[co->win makeKeyAndOrderFront:nil];
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static int
COCOA_LowerWindow(AG_Window *_Nonnull win)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	
	AG_MutexLock(&co->lock);
	if (![co->win isMiniaturized]) {
		[co->win orderBack:nil];
	}
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static int
COCOA_ReparentWindow(AG_Window *_Nonnull win, AG_Window *_Nonnull winParent,
    int x, int y)
{
	/* TODO */
	AG_SetError("Reparent window not implemented");
	return (-1);
}

static int
COCOA_GetInputFocus(AG_Window *_Nonnull *_Nonnull rv)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = NULL;

	AGOBJECT_FOREACH_CHILD(co, &agDrivers, ag_driver_cocoa) {
		if (!AGDRIVER_IS_COCOA(co)) {
			continue;
		}
		AG_MutexLock(&co->lock);
		if ([co->win isKeyWindow]) {
			AG_MutexUnlock(&co->lock);
			break;
		}
		AG_MutexUnlock(&co->lock);
	}
	if (co == NULL) {
		AG_SetError("Input focus is external to this application");
		goto fail;
	}
	*rv = AGDRIVER_MW(co)->win;
	[pool release];
	return (0);
fail:
	[pool release];
	return (-1);
}

static int
COCOA_SetInputFocus(AG_Window *_Nonnull win)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	
	AG_MutexLock(&co->lock);
	/* XXX use makeKeyWindow? */
	[co->win makeKeyAndOrderFront:nil];
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static void
COCOA_PreResizeCallback(AG_Window *_Nonnull win)
{
#if 0
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;

	/*
	 * Backup all GL resources since it is not portable to assume that a
	 * display resize will not cause a change in GL contexts
	 * (XXX TODO test for platforms where this is unnecessary)
	 * (XXX is this correctly done?)
	 */
	COCOA_GL_MakeCurrent(co);
	COCOA_FreeWidgetResources(WIDGET(win));
	AG_TextClearGlyphCache(co);
#endif
}

static void
COCOA_PostResizeCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverCocoa *co = (AG_DriverCocoa *)drv;
	AG_Rect r;
	NSRect trackRect;
	int x = a->x;
	int y = a->y;
	
	AG_MutexLock(&co->lock);

	a->x = 0;
	a->y = 0;
	AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0,0);

	/* The viewport coordinates have changed. */
	[co->glCtx makeCurrentContext];
	r.x = 0;
	r.y = 0;
	r.w = WIDTH(win);
	r.h = HEIGHT(win);
	AG_GL_SetViewport(&co->gl, &r);

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
	/* Update the tracking rectangle. */
	if (co->trackArea != nil) {
		[[co->win contentView] removeTrackingArea:co->trackArea];
		[co->trackArea release];
		co->trackArea = nil;
	}
	trackRect.origin.x = 0;
	trackRect.origin.y = 0;
	trackRect.size.width = WIDTH(win);
	trackRect.size.height = HEIGHT(win);
	co->trackArea = [[NSTrackingArea alloc] initWithRect:trackRect
	                 options: (NSTrackingMouseEnteredAndExited|
			           NSTrackingMouseMoved|
				   NSTrackingActiveAlways)
			 owner:co->win userInfo:nil];
	[[co->win contentView] addTrackingArea:co->trackArea];
#endif /* >= 10.5 */

	AG_MutexUnlock(&co->lock);

	/* Save the new effective window position. */
	WIDGET(win)->x = a->x = x;
	WIDGET(win)->y = a->y = y;
	
	[pool release];
}

static void
COCOA_PostMoveCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverCocoa *co = (AG_DriverCocoa *)drv;
	AG_SizeAlloc aNew;
	int xRel, yRel;
	
	AG_MutexLock(&co->lock);
	
	xRel = a->x - WIDGET(win)->x;
	yRel = a->y - WIDGET(win)->y;

	/* Update the window coordinates. */
	aNew.x = 0;
	aNew.y = 0;
	aNew.w = a->w;
	aNew.h = a->h;
	AG_WidgetSizeAlloc(win, &aNew);
	AG_WidgetUpdateCoords(win, 0,0);
	WIDGET(win)->x = a->x;
	WIDGET(win)->y = a->y;
	win->dirty = 1;

	if (agWindowPinnedCount > 0)
		AG_WindowMovePinned(win, xRel, yRel);

	AG_MutexUnlock(&co->lock);
}

static int
COCOA_MoveWindow(AG_Window *_Nonnull win, int x, int y)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	NSScreen *screen = [co->win screen];
	NSRect scrRect = [screen frame];
	NSPoint pt;

	pt.x = x;
	pt.y = scrRect.size.height - y;

	AG_MutexLock(&co->lock);
	[co->win setFrameTopLeftPoint:pt];
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

#if 0
/* Save/restore associated widget GL resources (for GL context changes). */
static void
COCOA_FreeWidgetResources(AG_Widget *_Nonnull wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		COCOA_FreeWidgetResources(chld);
	}
	AG_WidgetFreeResourcesGL(wid);
}
#endif

static int
COCOA_ResizeWindow(AG_Window *_Nonnull win, Uint w, Uint h)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	NSSize sz;

	sz.width = w;
	sz.height = h;

	AG_MutexLock(&co->lock);
	[co->win setContentSize:sz];
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static int
COCOA_MoveResizeWindow(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	NSScreen *screen = [co->win screen];
	NSRect scrRect = [screen frame];
	NSPoint pt;
	NSSize sz;

	pt.x = a->x;
	pt.y = scrRect.size.height - a->y;
	sz.width = a->w;
	sz.height = a->h;

	AG_MutexLock(&co->lock);
	[co->win setFrameTopLeftPoint:pt];
	[co->win setContentSize:sz];
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static int
COCOA_SetBorderWidth(AG_Window *_Nonnull win, Uint width)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;

	AG_MutexLock(&co->lock);
	if (([co->win styleMask] & NSTexturedBackgroundWindowMask) == 0) {
		[co->win setContentBorderThickness:width forEdge:NSMaxYEdge];
	}
	[co->win setContentBorderThickness:width forEdge:NSMinYEdge];
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static int
COCOA_SetWindowCaption(AG_Window *_Nonnull win, const char *_Nonnull s)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	NSString *string;
	
	if (win->caption[0] != '\0') {
		string = [[NSString alloc] initWithUTF8String:win->caption];
	} else {
		string = [[NSString alloc] init];
	}

	AG_MutexLock(&co->lock);
	[co->win setTitle:string];
	AG_MutexUnlock(&co->lock);

	[string release];
	[pool release];
	return (0);
}

static int
COCOA_SetOpacity(AG_Window *_Nonnull win, float f)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;

	AG_MutexLock(&co->lock);
	if (f > 0.99) {
		[co->win setOpaque:YES];
		[co->win setAlphaValue:1.0];
	} else {
		[co->win setOpaque:NO];
		[co->win setAlphaValue:f];
	}
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
}

static void
COCOA_TweakAlignment(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a,
    Uint wMax, Uint hMax)
{
	/* XXX TODO */
	switch (win->alignment) {
	case AG_WINDOW_TL:
	case AG_WINDOW_TC:
	case AG_WINDOW_TR:
		a->y += 50;
		if (a->y+a->h > hMax) { a->y = 0; }
		break;
	case AG_WINDOW_BL:
	case AG_WINDOW_BC:
	case AG_WINDOW_BR:
		a->y -= 100;
		if (a->y < 0) { a->y = 0; }
		break;
	default:
		break;
	}
}

static AG_Cursor *
COCOA_CreateCursor(void *_Nonnull obj, Uint w, Uint h, const Uint8 *_Nonnull data,
    const Uint8 *_Nonnull mask, int xHot, int yHot)
{
	AG_Cursor *ac;
	Uint size = w*h;

	if ((ac = TryMalloc(sizeof(AG_Cursor))) == NULL)
		return (NULL);
	if ((ac->data = TryMalloc(size)) == NULL)
		goto fail;
	if ((ac->mask = TryMalloc(size)) == NULL) {
		free(ac->data);
		goto fail;
	}
	memcpy(ac->data, data, size);
	memcpy(ac->mask, mask, size);
	ac->w = w;
	ac->h = h;
	ac->xHot = xHot;
	ac->yHot = yHot;

	/* TODO COCOA stuff here */
	return (NULL);
fail:
	free(ac);
	return (NULL);
}

static void
COCOA_FreeCursor(void *_Nonnull obj, AG_Cursor *_Nonnull ac)
{
	AG_Driver *drv = obj;

	if (ac == drv->activeCursor) {
		drv->activeCursor = NULL;
		/* TODO COCOA stuff here */
	}
	free(ac->data);
	free(ac->mask);
	free(ac);
}

static int
COCOA_SetCursor(void *_Nonnull obj, AG_Cursor *_Nonnull ac)
{
	AG_Driver *drv = obj;

	drv->activeCursor = ac;
	/* TODO COCOA stuff here */
	return (0);
}

static void
COCOA_UnsetCursor(void *_Nonnull obj)
{
	AG_Driver *drv = obj;

	if (drv->activeCursor == TAILQ_FIRST(&drv->cursors))
		return;
	
	/* TODO COCOA stuff here */
	drv->activeCursor = TAILQ_FIRST(&drv->cursors);		/* Default */
}

static int
COCOA_GetCursorVisibility(void *_Nonnull obj)
{
	/* TODO */
	return (1);
}

static void
COCOA_SetCursorVisibility(void *_Nonnull obj, int flag)
{
	/* TODO */
}

AG_DriverMwClass agDriverCocoa = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverCocoa",
			sizeof(AG_DriverCocoa),
			{ 1,6 },
			Init,
			NULL,		/* reset */
			Destroy,
			NULL,		/* load */
			NULL,		/* save */
			NULL,		/* edit */
		},
		"cocoa",
		AG_VECTOR,
		AG_WM_MULTIPLE,
		AG_DRIVER_OPENGL | AG_DRIVER_TEXTURES,
		COCOA_Open,
		COCOA_Close,
		COCOA_GetDisplaySize,
		NULL,			/* beginEventProcessing */
		COCOA_PendingEvents,
		COCOA_GetNextEvent,
		COCOA_ProcessEvent,
		NULL,			/* genericEventLoop */
		NULL,			/* endEventProcessing */
		NULL,			/* terminate */
		COCOA_BeginRendering,
		COCOA_RenderWindow,
		COCOA_EndRendering,
		AG_GL_FillRect,
		NULL,			/* updateRegion */
		AG_GL_StdUploadTexture,
		AG_GL_StdUpdateTexture,
		AG_GL_StdDeleteTexture,
		NULL,			/* setRefreshRate */
		AG_GL_StdPushClipRect,
		AG_GL_StdPopClipRect,
		AG_GL_StdPushBlendingMode,
		AG_GL_StdPopBlendingMode,
		COCOA_CreateCursor,
		COCOA_FreeCursor,
		COCOA_SetCursor,
		COCOA_UnsetCursor,
		COCOA_GetCursorVisibility,
		COCOA_SetCursorVisibility,
		AG_GL_BlitSurface,
		AG_GL_BlitSurfaceFrom,
#ifdef HAVE_OPENGL
		AG_GL_BlitSurfaceGL,
		AG_GL_BlitSurfaceFromGL,
		AG_GL_BlitSurfaceFlippedGL,
		AG_GL_BackupSurfaces,
		AG_GL_RestoreSurfaces,
#else
		NULL,                           /* backupSurfaces */
		NULL,                           /* restoreSurfaces */
#endif
		AG_GL_RenderToSurface,
		AG_GL_PutPixel,
		AG_GL_PutPixel32,
		AG_GL_PutPixelRGB8,
#if AG_MODEL == AG_LARGE
		AG_GL_PutPixel64,
		AG_GL_PutPixelRGB16,
#endif
		AG_GL_BlendPixel,
		AG_GL_DrawLine,
		AG_GL_DrawLineH,
		AG_GL_DrawLineV,
		AG_GL_DrawLineBlended,
		AG_GL_DrawLineW,
		AG_GL_DrawLineW_Sti16,
		AG_GL_DrawTriangle,
		AG_GL_DrawPolygon,
		AG_GL_DrawPolygon_Sti32,
		AG_GL_DrawArrow,
		AG_GL_DrawBoxRounded,
		AG_GL_DrawBoxRoundedTop,
		AG_GL_DrawCircle,
		AG_GL_DrawCircleFilled,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph,
		AG_GL_StdDeleteList,
		NULL,				/* getClipboardText */
		NULL				/* setClipboardText */
	},
	COCOA_OpenWindow,
	COCOA_CloseWindow,
	COCOA_MapWindow,
	COCOA_UnmapWindow,
	COCOA_RaiseWindow,
	COCOA_LowerWindow,
	COCOA_ReparentWindow,
	COCOA_GetInputFocus,
	COCOA_SetInputFocus,
	COCOA_MoveWindow,
	COCOA_ResizeWindow,
	COCOA_MoveResizeWindow,
	COCOA_PreResizeCallback,
	COCOA_PostResizeCallback,
	COCOA_SetBorderWidth,
	COCOA_SetWindowCaption,
	NULL,				/* setTransientFor */
	COCOA_SetOpacity,
	COCOA_TweakAlignment
};
