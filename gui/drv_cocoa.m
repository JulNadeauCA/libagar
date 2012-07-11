/*
 * Copyright (c) 2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include "gui.h"
#include "window.h"
#include "perfmon.h"
#include "gui_math.h"
#include "text.h"
#include "cursors.h"

#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLRenderers.h>

#include "drv_gl_common.h"
#include "drv_cocoa_keymap.h"

static int nDrivers = 0;		/* Drivers open */
static Uint rNom = 20;			/* Nominal refresh rate (ms) */
static int rCur = 0;			/* Effective refresh rate (ms) */
static int agExitCocoa = 0;
static AG_DriverEventQ agCocoaEventQ;	/* Private event queue */
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
	AG_Window *agarWindow;		/* Corresponding Agar window */
}

- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;
@end

@implementation AG_CocoaWindow
- (BOOL)canBecomeKeyWindow
{
	return YES;
}

- (BOOL)canBecomeMainWindow
{
	return YES;
}
@end

/*
 * View interface
 */
@interface AG_CocoaView : NSView
- (void)rightMouseDown:(NSEvent *)theEvent;
@end

@implementation AG_CocoaView
- (void)rightMouseDown:(NSEvent *)theEvent
{
	[[self nextResponder] rightMouseDown:theEvent];
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
	int              clipStates[4];	/* Clipping GL state */
	AG_ClipRect     *clipRects;	/* Clipping rectangles */
	Uint            nClipRects;
	Uint            *textureGC;	/* Textures queued for deletion */
	Uint            nTextureGC;
	Uint            *listGC;	/* Display lists queued for deletion */
	Uint            nListGC;
	AG_GL_BlendState bs[1];		/* Saved blending states */
	AG_Mutex         lock;		/* Protect Cocoa calls */
	Uint modFlags;			/* Last modifier state */
} AG_DriverCocoa;

AG_DriverMwClass agDriverCocoa;

#define AGDRIVER_IS_COCOA(drv) \
	(AGDRIVER_CLASS(drv) == (AG_DriverClass *)&agDriverCocoa)

static void COCOA_PostResizeCallback(AG_Window *, AG_SizeAlloc *);
static void COCOA_PostMoveCallback(AG_Window *, AG_SizeAlloc *);
static int  COCOA_RaiseWindow(AG_Window *);
static int  COCOA_SetInputFocus(AG_Window *);
#if 0
static void COCOA_FreeWidgetResources(AG_Widget *);
#endif

static __inline__ void
ConvertNSRect(NSRect *r)
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
	[win setAcceptsMouseMovedEvents:YES];

	[view setNextResponder:self];

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
	if ([view respondsToSelector:@selector(setAcceptsTouchEvents:)]) {
		[view setAcceptsTouchEvents:YES];
	}
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
	AG_PostEvent(NULL, _window, "window-close", NULL);
	return NO;
}

- (void)windowDidExpose:(NSNotification *)aNotification
{
	_window->dirty = 1;
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
	AG_DriverCocoa *co = _driver;
	AG_Window *win = _window;
	NSRect rect;
	AG_SizeAlloc a;

	rect = [co->win contentRectForFrameRect:[co->win frame]];
	ConvertNSRect(&rect);

	a.x = (int)rect.origin.x;
	a.y = (int)rect.origin.y;
	a.w = (int)rect.size.width;
	a.h = (int)rect.size.height;
	if (a.x != WIDGET(win)->x ||
	    a.y != WIDGET(win)->y) {
		COCOA_PostMoveCallback(win, &a);
	}
	COCOA_PostResizeCallback(win, &a);
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
	agWindowFocused = _window;
	AG_PostEvent(NULL, _window, "window-gainfocus", NULL);
}

- (void)windowDidResignKey:(NSNotification *)aNotification
{
	if (agWindowFocused == _window) {
		AG_PostEvent(NULL, _window, "window-lostfocus", NULL);
		agWindowFocused = NULL;
	}
}

@end

static void
Init(void *obj)
{
	AG_DriverCocoa *co = obj;

	co->clipRects = NULL;
	co->nClipRects = 0;
	memset(co->clipStates, 0, sizeof(co->clipStates));
	co->textureGC = NULL;
	co->nTextureGC = 0;
	co->listGC = NULL;
	co->nListGC = 0;
	co->win = NULL;
	co->glCtx = NULL;
	co->modFlags = 0;
	AG_MutexInitRecursive(&co->lock);
}

static void
Destroy(void *obj)
{
	AG_DriverCocoa *co = obj;

	AG_MutexDestroy(&co->lock);
	Free(co->clipRects);
	Free(co->textureGC);
	Free(co->listGC);
}

/*
 * Driver initialization
 */

static int
COCOA_Open(void *obj, const char *spec)
{
	AG_Driver *drv = obj;
	AG_DriverCocoa *co = obj;
	NSAutoreleasePool *pool;
	
	if (nDrivers == 0)
		TAILQ_INIT(&agCocoaEventQ);
	
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
    
	nDrivers++;
	return (0);
fail:
	if (drv->kbd != NULL) {
		AG_ObjectDetach(drv->kbd);
		AG_ObjectDestroy(drv->kbd);
		drv->kbd = NULL;
	}
	if (drv->mouse != NULL) {
		AG_ObjectDetach(drv->mouse);
		AG_ObjectDestroy(drv->mouse);
		drv->mouse = NULL;
	}
	return (-1);
}

static void
COCOA_Close(void *obj)
{
	AG_Driver *drv = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	if (--nDrivers == 0) {
		AG_DriverEvent *dev, *devNext;

		for (dev = TAILQ_FIRST(&agCocoaEventQ);
		     dev != TAILQ_LAST(&agCocoaEventQ, ag_driver_eventq);
		     dev = devNext) {
			devNext = TAILQ_NEXT(dev, events);
			Free(dev);
		}
		TAILQ_INIT(&agCocoaEventQ);
	}

	AG_ObjectDetach(drv->mouse);
	AG_ObjectDestroy(drv->mouse);
	AG_ObjectDetach(drv->kbd);
	AG_ObjectDestroy(drv->kbd);
	
	drv->mouse = NULL;
	drv->kbd = NULL;
}

static int
COCOA_GetDisplaySize(Uint *w, Uint *h)
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

static __inline__ int
COCOA_PendingEvents(void *drvCaller)
{
	NSAutoreleasePool *pool;
	NSEvent *ev;
	int rv;

	if (!TAILQ_EMPTY(&agCocoaEventQ))
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

static int
PostEventCallback(void *drvCaller)
{
	AG_Window *win;
	AG_Driver *drv;

	AG_LockVFS(&agDrivers);

	if (!TAILQ_EMPTY(&agWindowDetachQ))
		AG_FreeDetachedWindows();
	
	/*
	 * Exit when no more visible windows exist.
	 * XXX TODO make this behavior configurable
	 */
	if (TAILQ_EMPTY(&OBJECT(&agDrivers)->children)) {
		goto nowindows;
	}
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(win, drv) {
			if (win->visible)
				break;
		}
		if (win != NULL)
			break;
	}
	if (win == NULL)
		goto nowindows;

	if (agWindowToFocus != NULL) {
		AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(agWindowToFocus)->drv;

		if (co != NULL && AGDRIVER_IS_COCOA(co)) {
			AG_MutexLock(&co->lock);
			COCOA_RaiseWindow(agWindowToFocus);
			COCOA_SetInputFocus(agWindowToFocus);
			AG_MutexUnlock(&co->lock);
		}
		agWindowToFocus = NULL;
	}
	AG_UnlockVFS(&agDrivers);
	return (1);
nowindows:
	AG_SetError("No more windows exist");
	agTerminating = 1;
	AG_UnlockVFS(&agDrivers);
	return (-1);
}

/* Convert a NSEvent mouse button number to AG_MouseButton. */
static AG_MouseButton
GetMouseButton(int which)
{
	switch (which) {
	case 0:		return (AG_MOUSE_LEFT);
	case 1:		return (AG_MOUSE_RIGHT);
	case 2:		return (AG_MOUSE_MIDDLE);
	default:	return (which+1);
	}
}

static AG_MouseButton
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
QueueKeyEvent(AG_DriverCocoa *co, enum ag_driver_event_type type,
    AG_KeySym ks, Uint32 ucs)
{
	AG_DriverEvent *dev;

	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) == NULL) {
		AG_Verbose("Out of memory for keymod event\n");
		return;
	}
	dev->win = AGDRIVER_MW(co)->win;
	dev->type = type;
	dev->data.key.ks = ks;
	dev->data.key.ucs = ucs;
	TAILQ_INSERT_TAIL(&agCocoaEventQ, dev, events);
}

static int
COCOA_GetNextEvent(void *drvCaller, AG_DriverEvent *dev)
{
	NSAutoreleasePool *pool;
	AG_CocoaWindow *coWin;
	AG_Window *win;
	AG_Driver *drv;
	AG_DriverCocoa *co;
	AG_DriverEvent *devFirst;
	NSEvent *event;
	int rv = 0;
	
	if (!TAILQ_EMPTY(&agCocoaEventQ))
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
	win = coWin->agarWindow;
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
		dev->type = AG_DRIVER_MOUSE_ENTER;
		dev->win = win;
		rv = 1;
		break;
	case NSMouseExited:
		dev->type = AG_DRIVER_MOUSE_LEAVE;
		dev->win = win;
		rv = 1;
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
				AG_KeyboardUpdate(drv->kbd, kbdAction, ks, 0);
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
				AG_KeyboardUpdate(drv->kbd, kbdAction, ks,
				    (Uint32)c);

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
					AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED, kmEnt->keySym, 0);
					QueueKeyEvent(co, AG_KEY_PRESSED, kmEnt->keySym, 0);
					nChanged++;
				} else if (!(modFlags & kmEnt->keyMask) &&
				    (co->modFlags & kmEnt->keyMask)) {
					AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED, kmEnt->keySym, 0);
					QueueKeyEvent(co, AG_KEY_RELEASED, kmEnt->keySym, 0);
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
	devFirst = TAILQ_FIRST(&agCocoaEventQ);
	TAILQ_REMOVE(&agCocoaEventQ, devFirst, events);
	memcpy(dev, devFirst, sizeof(AG_DriverEvent));
	Free(devFirst);
	return (1);
}

static int
COCOA_ProcessEvent(void *drvCaller, AG_DriverEvent *dev)
{
	AG_Driver *drv;
	AG_SizeAlloc a;

	if (dev->win == NULL) {
		return (0);
	}
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		if (WIDGET(dev->win)->drv == drv)
			break;
	}
	if (drv == NULL) {
		return (0);
	}
	drv = WIDGET(dev->win)->drv;

	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		AG_ProcessMouseMotion(dev->win,
		    dev->data.motion.x, dev->data.motion.y,
		    drv->mouse->xRel, drv->mouse->yRel,
		    drv->mouse->btnState);
		AG_MouseCursorUpdate(dev->win,
		     dev->data.motion.x, dev->data.motion.y);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		AG_ProcessMouseButtonDown(dev->win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		AG_ProcessMouseButtonUp(dev->win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_KEY_UP:
		AG_ProcessKey(drv->kbd, dev->win, AG_KEY_RELEASED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_KEY_DOWN:
		AG_ProcessKey(drv->kbd, dev->win, AG_KEY_PRESSED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_MOUSE_ENTER:
		AG_PostEvent(NULL, dev->win, "window-enter", NULL);
		break;
	case AG_DRIVER_MOUSE_LEAVE:
		AG_PostEvent(NULL, dev->win, "window-leave", NULL);
		break;
	case AG_DRIVER_FOCUS_IN:
		agWindowFocused = dev->win;
		AG_PostEvent(NULL, dev->win, "window-gainfocus", NULL);
		break;
	case AG_DRIVER_FOCUS_OUT:
		if (dev->win == agWindowFocused) {
			AG_PostEvent(NULL, dev->win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		break;
	case AG_DRIVER_VIDEORESIZE:
		a.x = dev->data.videoresize.x;
		a.y = dev->data.videoresize.y;
		a.w = dev->data.videoresize.w;
		a.h = dev->data.videoresize.h;
		if (a.w != WIDTH(dev->win) || a.h != HEIGHT(dev->win)) {
			COCOA_PostResizeCallback(dev->win, &a);
		} else {
			COCOA_PostMoveCallback(dev->win, &a);
		}
		break;
	case AG_DRIVER_CLOSE:
		AG_PostEvent(NULL, dev->win, "window-close", NULL);
		break;
	case AG_DRIVER_EXPOSE:
		dev->win->dirty = 1;
		break;
	default:
		break;
	}
	return PostEventCallback(drvCaller);
}

static void
COCOA_GenericEventLoop(void *obj)
{
	AG_Driver *drv;
	AG_DriverCocoa *co;
	AG_DriverEvent dev;
	AG_Window *win;
	Uint32 t1, t2;

#ifdef AG_DEBUG
	AG_PerfMonInit();
#endif
	t1 = AG_GetTicks();
	for (;;) {
		t2 = AG_GetTicks();
		if (agExitCocoa) {
			break;
		} else if (t2 - t1 >= rNom) {
			AG_LockVFS(&agDrivers);
			AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
				if (!AGDRIVER_IS_COCOA(drv)) {
					continue;
				}
				co = (AG_DriverCocoa *)drv;
				AG_MutexLock(&co->lock);
				win = AGDRIVER_MW(drv)->win;
				if (win->visible && win->dirty) {
					AG_BeginRendering(drv);
					AG_ObjectLock(win);
					AG_WindowDraw(win);
					AG_ObjectUnlock(win);
					AG_EndRendering(drv);
				}
				AG_MutexUnlock(&co->lock);
			}
			AG_UnlockVFS(&agDrivers);
			t1 = AG_GetTicks();
			rCur = rNom - (t1-t2);
			if (rCur < 1) { rCur = 1; }
#ifdef AG_DEBUG
			if (agPerfWindow->visible)
				AG_PerfMonUpdate(rCur);
#endif
		} else if (COCOA_GetNextEvent(NULL, &dev) == 1) {
			if (COCOA_ProcessEvent(NULL, &dev) == -1)
				return;
#ifdef AG_DEBUG
			agEventAvg++;
#endif
		} else if (AG_TIMEOUTS_QUEUED()) {		/* Safe */
			AG_ProcessTimeouts(t2);
		} else {
			AG_Delay(1);
		}
	}
}

static void
COCOA_Terminate(void)
{
	agExitCocoa = 1;
}

/* Select the window's OpenGL context. */
static __inline__ void
COCOA_GL_MakeCurrent(AG_DriverCocoa *co, AG_Window *win)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (win->visible) {
		[co->glCtx setView:[co->win contentView]];
		[co->glCtx update];
	}
	[co->glCtx makeCurrentContext];

	[pool release];
}

static void
COCOA_BeginRendering(void *obj)
{
	AG_DriverCocoa *co = obj;

	COCOA_GL_MakeCurrent(co, AGDRIVER_MW(co)->win);
}

static void
COCOA_RenderWindow(AG_Window *win)
{
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;

	co->clipStates[0] = glIsEnabled(GL_CLIP_PLANE0);
	glEnable(GL_CLIP_PLANE0);
	co->clipStates[1] = glIsEnabled(GL_CLIP_PLANE1);
	glEnable(GL_CLIP_PLANE1);
	co->clipStates[2] = glIsEnabled(GL_CLIP_PLANE2);
	glEnable(GL_CLIP_PLANE2);
	co->clipStates[3] = glIsEnabled(GL_CLIP_PLANE3);
	glEnable(GL_CLIP_PLANE3);

	/* clear the clipped erea with the background colour */
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	AG_WidgetDraw(win);
}

static void
COCOA_EndRendering(void *obj)
{
	AG_DriverCocoa *co = obj;
	Uint i;

	[co->glCtx flushBuffer];

	/* Remove textures and display lists queued for deletion. */
	glDeleteTextures(co->nTextureGC, (const GLuint *)co->textureGC);
	for (i = 0; i < co->nListGC; i++) {
		glDeleteLists(co->listGC[i], 1);
	}
	co->nTextureGC = 0;
	co->nListGC = 0;
}

static void
COCOA_DeleteTexture(void *drv, Uint texture)
{
	AG_DriverCocoa *co = drv;

	co->textureGC = Realloc(co->textureGC, (co->nTextureGC+1)*sizeof(Uint));
	co->textureGC[co->nTextureGC++] = texture;
}

static void
COCOA_DeleteList(void *drv, Uint list)
{
	AG_DriverCocoa *co = drv;

	co->listGC = Realloc(co->listGC, (co->nListGC+1)*sizeof(Uint));
	co->listGC[co->nListGC++] = list;
}

static int
COCOA_SetRefreshRate(void *obj, int fps)
{
	if (fps < 1) {
		AG_SetError("Invalid refresh rate");
		return (-1);
	}
	rNom = 1000/fps;
	rCur = 0;
	return (0);
}

/*
 * Clipping and blending control (rendering context)
 */

static void
COCOA_PushClipRect(void *obj, AG_Rect r)
{
	AG_DriverCocoa *co = obj;
	AG_ClipRect *cr, *crPrev;

	AG_MutexLock(&co->lock);

	co->clipRects = Realloc(co->clipRects, (co->nClipRects+1)*sizeof(AG_ClipRect));
	crPrev = &co->clipRects[co->nClipRects-1];
	cr = &co->clipRects[co->nClipRects++];

	cr->eqns[0][0] = 1.0;
	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;
	cr->eqns[0][3] = MIN(crPrev->eqns[0][3], -(double)(r.x));
	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	
	cr->eqns[1][0] = 0.0;
	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;
	cr->eqns[1][3] = MIN(crPrev->eqns[1][3], -(double)(r.y));
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
		
	cr->eqns[2][0] = -1.0;
	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;
	cr->eqns[2][3] = MIN(crPrev->eqns[2][3], (double)(r.x+r.w));
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
		
	cr->eqns[3][0] = 0.0;
	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;
	cr->eqns[3][3] = MIN(crPrev->eqns[3][3], (double)(r.y+r.h));
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
	
	AG_MutexUnlock(&co->lock);
}

static void
COCOA_PopClipRect(void *obj)
{
	AG_DriverCocoa *co = obj;
	AG_ClipRect *cr;
	
	AG_MutexLock(&co->lock);
#ifdef AG_DEBUG
	if (co->nClipRects < 1)
		AG_FatalError("PopClipRect() without PushClipRect()");
#endif
	cr = &co->clipRects[co->nClipRects-2];
	co->nClipRects--;

	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
	
	AG_MutexUnlock(&co->lock);
}

static void
COCOA_PushBlendingMode(void *obj, AG_BlendFn fnSrc, AG_BlendFn fnDst)
{
	AG_DriverCocoa *co = obj;
	
	AG_MutexLock(&co->lock);

	/* XXX TODO: stack */
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &co->bs[0].texEnvMode);
	glGetBooleanv(GL_BLEND, &co->bs[0].enabled);
	glGetIntegerv(GL_BLEND_SRC, &co->bs[0].srcFactor);
	glGetIntegerv(GL_BLEND_DST, &co->bs[0].dstFactor);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(AG_GL_GetBlendingFunc(fnSrc), AG_GL_GetBlendingFunc(fnDst));
	
	AG_MutexUnlock(&co->lock);
}

static void
COCOA_PopBlendingMode(void *obj)
{
	AG_DriverCocoa *co = obj;
	
	AG_MutexLock(&co->lock);

	/* XXX TODO: stack */
	if (co->bs[0].enabled) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
	glBlendFunc(co->bs[0].srcFactor, co->bs[0].dstFactor);
	
	AG_MutexUnlock(&co->lock);
}

/*
 * Window operations
 */

/* Initialize the clipping rectangle stack. */
static int
InitClipRects(AG_DriverCocoa *co, int w, int h)
{
	AG_ClipRect *cr;
	int i;

	for (i = 0; i < 4; i++)
		co->clipStates[i] = 0;

	/* Rectangle 0 always covers the whole view. */
	if ((co->clipRects = TryMalloc(sizeof(AG_ClipRect))) == NULL) {
		return (-1);
	}
	co->nClipRects = 1;

	cr = &co->clipRects[0];
	cr->r = AG_RECT(0,0, w,h);

	cr->eqns[0][0] = 1.0;	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;	cr->eqns[0][3] = 0.0;
	cr->eqns[1][0] = 0.0;	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;	cr->eqns[1][3] = 0.0;
	cr->eqns[2][0] = -1.0;	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;	cr->eqns[2][3] = (double)w;
	cr->eqns[3][0] = 0.0;	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;	cr->eqns[3][3] = (double)h;

	return (0);
}

/* Initialize clipping rectangle 0 for the current window geometry. */
static void
InitClipRect0(AG_DriverCocoa *co, AG_Window *win)
{
	AG_ClipRect *cr;

	cr = &co->clipRects[0];
	cr->r.w = WIDTH(win);
	cr->r.h = HEIGHT(win);
	cr->eqns[2][3] = (double)WIDTH(win);
	cr->eqns[3][3] = (double)HEIGHT(win);
}

static int
COCOA_OpenWindow(AG_Window *win, AG_Rect r, int depthReq, Uint mwFlags)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv;
	Uint winStyle = 0;
	NSRect winRect, scrRect;
	NSArray *screens;
	NSScreen *screen, *selScreen = nil;
	NSView *contentView;
	NSOpenGLPixelFormatAttribute pfAttr[32];
	NSOpenGLPixelFormat *pf;
	int i, count;
	AG_SizeAlloc a;

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
	winRect.origin.x = r.x;
	winRect.origin.y = r.y;
	winRect.size.width = r.w;
	winRect.size.height = r.h;
	ConvertNSRect(&winRect);

	/* Select a screen. */
	screens = [NSScreen screens];
	count = [screens count];
	for (i = 0; i < count; i++) {
		screen = [screens objectAtIndex:i];
		scrRect = [screen frame];

		if (winRect.origin.x >= scrRect .origin.x &&
		    winRect.origin.y >= scrRect .origin.y &&
		    winRect.origin.x < scrRect.origin.x + scrRect.size.width &&
		    winRect.origin.y < scrRect.origin.y + scrRect.size.height) {
		    	selScreen = screen;
			winRect.origin.x -= scrRect.origin.x;
			winRect.origin.y -= scrRect.origin.y;
			break;
		}
	}
	if (selScreen == nil) {
		AG_MutexUnlock(&co->lock);
		[pool release];
		AG_SetError("Failed to select screen");
		return (-1);
	}

	/* Create the window. */
	co->win = [[AG_CocoaWindow alloc] initWithContentRect:winRect
	                                  styleMask:winStyle
				          backing:NSBackingStoreBuffered
				          defer:YES
				          screen:selScreen];
	co->win->agarWindow = win;

	/* Create an event listener. */
	co->evListener = [[AG_CocoaListener alloc] init];

	/* Obtain the effective window coordinates; create the NSView. */
	winRect = [co->win contentRectForFrameRect:[co->win frame]];
	contentView = [co->win contentView];
	if (!contentView) {
		contentView = [[AG_CocoaView alloc] initWithFrame:winRect];
		[co->win setContentView: contentView];
		[contentView release];
	}
	ConvertNSRect(&winRect);

	/* Set our event listener. */
	[co->evListener listen:co];

	/* Get the visibility status. */
	win->visible = [co->win isVisible];

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
	if (depthReq != 0) {
		pfAttr[i++] = NSOpenGLPFADepthSize;
		pfAttr[i++] = depthReq;
	}
	pfAttr[i++] = NSOpenGLPFADoubleBuffer;
#if 0
	pfAttr[i++] = NSOpenGLPFAScreenMask;
	pfAttr[i++] = CGDisplayIDToOpenGLDisplayMask(display);
#endif
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
	
	AGDRIVER_MW(co)->flags |= AG_DRIVER_MW_OPEN;

	/* Set the preferred Agar pixel formats. */
	/* XXX XXX XXX: retrieve effective depth */
	drv->videoFmt = AG_PixelFormatRGB(depthReq != 0 ? depthReq : 16,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		0xff000000, 0x00ff0000, 0x0000ff00
#else
		0x000000ff, 0x0000ff00, 0x00ff0000
#endif
	);
	if (drv->videoFmt == NULL)
		goto fail;

	/* Initialize the clipping rectangle stack. */
	if (InitClipRects(co, r.w, r.h) == -1)
		goto fail;
	
	/* Set the effective window geometry, initialize the GL context. */
	a.x = winRect.origin.x;
	a.y = winRect.origin.y;
	a.w = winRect.size.width;
	a.h = winRect.size.height;
	COCOA_PostResizeCallback(win, &a);
	
	AG_MutexUnlock(&co->lock);
	[pool release];
	return (0);
fail:
	[NSOpenGLContext clearCurrentContext];
	[co->evListener close];
	[co->evListener release];
	[co->win close];
	AGDRIVER_MW(co)->flags &= ~(AG_DRIVER_MW_OPEN);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	AG_MutexUnlock(&co->lock);
	[pool release];
	return (-1);
}

static void
COCOA_CloseWindow(AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverCocoa *co = (AG_DriverCocoa *)drv;
/*	AG_Glyph *gl; */
/*	int i; */

	AG_MutexLock(&co->lock);
#if 0
	/* Invalidate cached glyph textures. */
	COCOA_GL_MakeCurrent(co, win)
	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		SLIST_FOREACH(gl, &drv->glyphCache[i].glyphs, glyphs) {
			if (gl->texture != 0) {
				glDeleteTextures(1, (GLuint *)&gl->texture);
				gl->texture = 0;
			}
		}
	}
#endif
	[co->glCtx clearDrawable];
	[co->glCtx release];
	[co->evListener close];
	[co->evListener release];
	[co->win close];
	AG_PixelFormatFree(drv->videoFmt);
	drv->videoFmt = NULL;
	AGDRIVER_MW(co)->flags &= ~(AG_DRIVER_MW_OPEN);
	AG_MutexUnlock(&co->lock);
}

static int
COCOA_MapWindow(AG_Window *win)
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
COCOA_UnmapWindow(AG_Window *win)
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
COCOA_RaiseWindow(AG_Window *win)
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
COCOA_LowerWindow(AG_Window *win)
{
	/* TODO */
	AG_SetError("Lower window not implemented");
	return (-1);
}

static int
COCOA_ReparentWindow(AG_Window *win, AG_Window *winParent, int x, int y)
{
	/* TODO */
	AG_SetError("Reparent window not implemented");
	return (-1);
}

static int
COCOA_GetInputFocus(AG_Window **rv)
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
COCOA_SetInputFocus(AG_Window *win)
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
COCOA_PreResizeCallback(AG_Window *win)
{
#if 0
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;

	/*
	 * Backup all GL resources since it is not portable to assume that a
	 * display resize will not cause a change in GL contexts
	 * (XXX TODO test for platforms where this is unnecessary)
	 * (XXX is this correctly done?)
	 */
	COCOA_GL_MakeCurrent(co, win);
	COCOA_FreeWidgetResources(WIDGET(win));
	AG_TextClearGlyphCache(co);
#endif
}

static void
COCOA_PostResizeCallback(AG_Window *win, AG_SizeAlloc *a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverCocoa *co = (AG_DriverCocoa *)drv;
	int x = a->x;
	int y = a->y;
	
	AG_MutexLock(&co->lock);

	/* Update per-widget coordinate information. */
	a->x = 0;
	a->y = 0;
	(void)AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0, 0);

	/* Update clipping rectangle 0 */
	InitClipRect0(co, win);

	/* Update OpenGL context. */
	COCOA_GL_MakeCurrent(co, win);
	AG_GL_InitContext(AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
	
	AG_MutexUnlock(&co->lock);

	/* Save the new effective window position. */
	WIDGET(win)->x = a->x = x;
	WIDGET(win)->y = a->y = y;
}

static void
COCOA_PostMoveCallback(AG_Window *win, AG_SizeAlloc *a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverCocoa *co = (AG_DriverCocoa *)drv;
	int x = a->x;
	int y = a->y;

	/* Update per-widget coordinate information. */
	a->x = 0;
	a->y = 0;
	(void)AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0, 0);

	/* Update OpenGL context. */
	COCOA_GL_MakeCurrent(co, win);
	AG_GL_InitContext(AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
	
	/* Save the new effective window position. */
	WIDGET(win)->x = a->x = x;
	WIDGET(win)->y = a->y = y;
}

static int
COCOA_MoveWindow(AG_Window *win, int x, int y)
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
COCOA_FreeWidgetResources(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		COCOA_FreeWidgetResources(chld);
	}
	AG_WidgetFreeResourcesGL(wid);
}
#endif

static int
COCOA_ResizeWindow(AG_Window *win, Uint w, Uint h)
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
COCOA_MoveResizeWindow(AG_Window *win, AG_SizeAlloc *a)
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
COCOA_SetBorderWidth(AG_Window *win, Uint width)
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
COCOA_SetWindowCaption(AG_Window *win, const char *s)
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
COCOA_SetOpacity(AG_Window *win, float f)
{
#if 0
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	AG_DriverCocoa *co = (AG_DriverCocoa *)WIDGET(win)->drv;
	AG_Color bgColor = agColors[BG_COLOR];
	CGFloat r, g, b;

	r = (CGFloat)(bgColor.r / 255.0);
	g = (CGFloat)(bgColor.g / 255.0);
	b = (CGFloat)(bgColor.b / 255.0);

	AG_MutexLock(&co->lock);
	if (f > 0.99) {
		[co->win setOpaque:YES];
	} else {
		[co->win setOpaque:NO];
		[co->win setBackgroundColor:[NSColor colorWithSRGBRed:r green:g blue:b alpha:f]];
	}
	AG_MutexUnlock(&co->lock);

	[pool release];
	return (0);
#endif
	AG_SetError("Opacity not supported");
	return (-1);
}

AG_DriverMwClass agDriverCocoa = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverCocoa",
			sizeof(AG_DriverCocoa),
			{ 1,4 },
			Init,
			NULL,	/* reinit */
			Destroy,
			NULL,	/* load */
			NULL,	/* save */
			NULL,	/* edit */
		},
		"cocoa",
		AG_VECTOR,
		AG_WM_MULTIPLE,
		AG_DRIVER_OPENGL|AG_DRIVER_TEXTURES,
		COCOA_Open,
		COCOA_Close,
		COCOA_GetDisplaySize,
		NULL,			/* beginEventProcessing */
		COCOA_PendingEvents,
		COCOA_GetNextEvent,
		COCOA_ProcessEvent,
		COCOA_GenericEventLoop,
		NULL,			/* endEventProcessing */
		COCOA_Terminate,
		COCOA_BeginRendering,
		COCOA_RenderWindow,
		COCOA_EndRendering,
		AG_GL_FillRect,
		NULL,			/* updateRegion */
		AG_GL_UploadTexture,
		AG_GL_UpdateTexture,
		COCOA_DeleteTexture,
		COCOA_SetRefreshRate,
		COCOA_PushClipRect,
		COCOA_PopClipRect,
		COCOA_PushBlendingMode,
		COCOA_PopBlendingMode,
		NULL,			/* createCursor */
		NULL,			/* freeCursor */
		NULL,			/* setCursor */
		NULL,			/* unsetCursor */
		NULL,			/* getCursorVisibility */
		NULL,			/* setCursorVisibility */
		AG_GL_BlitSurface,
		AG_GL_BlitSurfaceFrom,
		AG_GL_BlitSurfaceGL,
		AG_GL_BlitSurfaceFromGL,
		AG_GL_BlitSurfaceFlippedGL,
		AG_GL_BackupSurfaces,
		AG_GL_RestoreSurfaces,
		AG_GL_RenderToSurface,
		AG_GL_PutPixel,
		AG_GL_PutPixel32,
		AG_GL_PutPixelRGB,
		AG_GL_BlendPixel,
		AG_GL_DrawLine,
		AG_GL_DrawLineH,
		AG_GL_DrawLineV,
		AG_GL_DrawLineBlended,
		AG_GL_DrawArrowUp,
		AG_GL_DrawArrowDown,
		AG_GL_DrawArrowLeft,
		AG_GL_DrawArrowRight,
		AG_GL_DrawBoxRounded,
		AG_GL_DrawBoxRoundedTop,
		AG_GL_DrawCircle,
		AG_GL_DrawCircle2,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph,
		COCOA_DeleteList
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
	NULL,				/* captureWindow */
	COCOA_SetBorderWidth,
	COCOA_SetWindowCaption,
	NULL,				/* setTransientFor */
	COCOA_SetOpacity
};
