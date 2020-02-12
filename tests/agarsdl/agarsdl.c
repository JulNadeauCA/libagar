/*	Public domain	*/
/*
 * Test functionality specific to SDL, notably attaching to an existing
 * SDL display surface, and converting a SDL_Surface to an AG_Surface.
 */

#include <agar/config/have_opengl.h>

#include <SDL.h>
#include <math.h>

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>

SDL_Surface *screen;			/* Our existing SDL display surface */
int curFPS = 0;
float color = 0.5;
int Width = 320;
int Height = 240;

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

		if (t2-t1 >= 1000/myFPS) {
			/*
			 * Insert arbitrary OpenGL code here
			 */
//			glClearColor(0,0,color,1);
//			glClear(GL_COLOR_BUFFER_BIT);

			/* Render Agar GUI windows */
			AG_FOREACH_WINDOW(win, drv) {
				if (win->visible && win->dirty)
					break;
			}
			if (win != NULL) {
				AG_BeginRendering(drv);
				AG_FOREACH_WINDOW(win, drv) {
					AG_ObjectLock(win);
					AG_WindowDraw(win);
					AG_ObjectUnlock(win);
				}
				AG_EndRendering(drv);
			}

			SDL_GL_SwapBuffers();
			
			t1 = AG_GetTicks();
			curFPS = 1000/myFPS - (t1-t2);
			if (curFPS < 1) { curFPS = 1; }
		} else if (AG_PendingEvents(NULL)) {
			if (AG_GetNextEvent(drv, &dev) == 1) {
				if (dev.type == AG_DRIVER_KEY_DOWN) {
					switch (dev.data.key.ks) {
					case AG_KEY_ESCAPE:
						return;
					case AG_KEY_UP:
						color -= 0.1;
						break;
					case AG_KEY_DOWN:
						color += 0.1;
						break;
					default:
						break;
					}
				}
			}
			if (AG_ProcessEvent(drv, &dev) == -1) {
				return;
			}
		} else {
			/* Expire software timers. */
			AG_ProcessTimeouts(t2);
			AG_Delay(1);
		}
		AG_WindowProcessQueued();
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

	/*
	 * Note: For the AG_ProcessTimeouts() call in custom event loop to be safe,
	 * we must request software timers with AG_SOFT_TIMERS.
	 */
	if (AG_InitCore(NULL, AG_VERBOSE|AG_SOFT_TIMERS) == -1) {
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
			printf("Usage: agarsdl [-go]\n");
			printf("\n");
			printf("\t[-g]: Attach to existing OpenGL context\n");
			printf("\t[-o]: Set AG_VIDEO_OVERLAY mode\n");
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
		sdlFlags = SDL_OPENGL;
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	} else {
		printf("Attaching to software surface\n");
		sdlFlags = SDL_SWSURFACE;
	}
	if ((screen = SDL_SetVideoMode(Width, Height, 16, sdlFlags)) == NULL) {
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		goto fail;
	}
	if (ivFlags & AG_VIDEO_OPENGL) {
		glViewport(0, 0, Width, Height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, Width, Height, 0, -1.0, 1.0);

		glClearColor(1,0,0,1); glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapBuffers(); AG_Delay(300);
		glClearColor(0,1,0,1); glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapBuffers(); AG_Delay(300);
		glClearColor(0,0,1,1); glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapBuffers();
		AG_Delay(1000);
	}

	/* Agar requires Unicode key events. */
	SDL_EnableUNICODE(1);

	/* Initialize Agar-GUI to reuse display */
	printf("Initializing Agar GUI\n");
	if (AG_InitVideoSDL(screen, ivFlags) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		goto fail;
	}
	AG_BindStdGlobalKeys();
	AG_BindGlobalKey(AG_KEY_ESCAPE, 0, AG_QuitGUI);
	
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
