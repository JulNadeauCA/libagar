/*	Public domain	*/
/*
 * Test functionality specific to SDL, notably attaching to an existing
 * SDL display surface, and converting a SDL_Surface to an AG_Surface.
 */

#include <agar/config/have_opengl.h>

#include <SDL.h>

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>

SDL_Surface *screen;			/* Our existing SDL display surface */
int curFPS = 0;

/* Custom event loop routine to test AG_VIDEO_OVERLAY mode. */
static void
MyEventLoop_Overlay(int myFPS)
{
	AG_Driver *drv = (AG_Driver *)agDriverSw;
	AG_Window *win;
	Uint32 t1, t2;
	AG_DriverEvent dev;

	t1 = AG_GetTicks();
	for (;;) {
		t2 = AG_GetTicks();

		if (t2-t1 >= myFPS) {
			AG_LockVFS(&agDrivers);
			AG_FOREACH_WINDOW(win, agDriverSw) {
				if (win->dirty)
					break;
			}
			if (win != NULL) {
				/* Arbitrary OpenGL code */
				glClearColor(0, 0, 0.8, 1.0);
				glClear(GL_COLOR_BUFFER_BIT);

				/* Render Agar GUI windows */
				AG_BeginRendering(agDriverSw);
				AG_FOREACH_WINDOW(win, agDriverSw) {
					if (!win->visible) {
						continue;
					}
					AG_ObjectLock(win);
					AG_WindowDraw(win);
					AG_ObjectUnlock(win);
				}
				AG_EndRendering(agDriverSw);

				/* For double-buffering */
				SDL_GL_SwapBuffers();
			}
			AG_UnlockVFS(&agDrivers);
			t1 = AG_GetTicks();
			curFPS = myFPS - (t1-t2);
			if (curFPS < 1) { curFPS = 1; }
		} else if (SDL_PollEvent(NULL) != 0) {
			if (AG_GetNextEvent(drv, &dev) == 1 &&
			    AG_ProcessEvent(drv, &dev) == -1) {
				return;
			}
		} else if (AG_TIMEOUTS_QUEUED()) {
			AG_ProcessTimeouts(t2);
		} else {
			AG_Delay(1);
		}
	}
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	SDL_Surface *bmp;
	AG_Surface *agbmp;
	Uint32 sdlFlags = 0;
	char *optArg;
	int ivFlags = 0;
	int c;

	if (AG_InitCore(NULL, AG_VERBOSE) == -1) {
		fprintf(stderr, "AG_InitCore: %s\n", AG_GetError());
		return (1);
	}
	while ((c = AG_Getopt(argc, argv, "?go", &optArg, NULL)) != -1) {
		switch (c) {
		case 'g':
			ivFlags |= AG_VIDEO_OPENGL;
			break;
		case 'o':
			ivFlags |= AG_VIDEO_OVERLAY;
			break;
		default:
			printf("Usage: agarsdl [-g]\n");
			break;
		}
	}

	/* Set up SDL */
	printf("Initializing SDL\n");
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return (1);
	}
	if (ivFlags & AG_VIDEO_OPENGL) {
		printf("Attaching to OpenGL context\n");
		sdlFlags = SDL_RESIZABLE|SDL_OPENGL;
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	} else {
		printf("Attaching to software surface\n");
		sdlFlags = SDL_RESIZABLE|SDL_SWSURFACE;
	}
	if ((screen = SDL_SetVideoMode(320, 240, 32, sdlFlags)) == NULL) {
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		goto fail;
	}

	/* Agar requires Unicode key events. */
	SDL_EnableUNICODE(1);

	/* Initialize Agar-GUI to reuse display */
	printf("Initializing Agar GUI\n");
	if (AG_InitVideoSDL(screen, ivFlags) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		goto fail;
	}
#ifdef __APPLE__
	AG_BindGlobalKey(AG_KEY_Q, AG_KEYMOD_META, AG_QuitGUI);
#else
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
#endif
	
	/* Display some test widgets. */
	win = AG_WindowNew(0);
	AG_LabelNew(win, 0, "Attached to existing %s display",
	    (ivFlags & AG_VIDEO_OPENGL) ? "SDL/OpenGL" : "SDL");
	if (ivFlags & AG_VIDEO_OVERLAY) {
		AG_Label *lbl;
		AG_LabelNew(win, 0, "Running in AG_VIDEO_OVERLAY mode");
		lbl = AG_LabelNewPolled(win, 0, "Frame rate: %i fps", &curFPS);
		AG_LabelSizeHint(lbl, 1, "Frame rate: XXXXXXX");
	}

	/* Test conversion from SDL_Surface to AG_Surface. */
	if ((bmp = SDL_LoadBMP("agar.bmp")) != NULL) {
		if ((agbmp = AG_SurfaceFromSDL(bmp)) != NULL) {
			AG_PixmapFromSurface(win, 0, agbmp);
		} else {
			AG_LabelNewS(win, 0, AG_GetError());
		}
	}

	AG_TextboxNew(win, AG_TEXTBOX_HFILL, "Some text: ");
	AG_ButtonNew(win, AG_BUTTON_HFILL, "Test GUI responsiveness");

	AG_WindowShow(win);
	
	if (ivFlags & AG_VIDEO_OVERLAY) {
		MyEventLoop_Overlay(30);
	} else {
		AG_EventLoop();
	}
	AG_Destroy();
	SDL_Quit();
	return (0);
fail:
	AG_Destroy();
	SDL_Quit();
	return (1);
}
