/*	Public domain	*/
/*
 * Test functionality specific to SDL, notably attaching to an existing
 * SDL display surface, and converting a SDL_Surface to an AG_Surface.
 */

#include <agar/config/have_sdl.h>
#include <agar/config/have_opengl.h>
#ifdef HAVE_SDL
#include <SDL.h>
#endif
#ifdef HAVE_OPENGL
#include <agar/gui/opengl.h>
#endif

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
#ifdef HAVE_SDL
	AG_Window *win;
	SDL_Surface *screen, *bmp;
	AG_Surface *agbmp;
	int c, useGL = 0;
	Uint32 sdlFlags = 0;
	char *optArg;
	
	if (AG_InitCore("agar-sdl-demo", 0) == -1) {
		fprintf(stderr, "AG_InitCore: %s\n", AG_GetError());
		goto fail;
	}
	while ((c = AG_Getopt(argc, argv, "?g", &optArg, NULL)) != -1) {
		switch (c) {
		case 'g':
			useGL = 1;
			break;
		default:
			printf("Usage: %s [-g]\n", agProgName);
			break;
		}
	}

	/* Set up SDL */
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return (1);
	}
	if (useGL) {
		sdlFlags = SDL_RESIZABLE|SDL_OPENGL;
	} else {
		sdlFlags = SDL_RESIZABLE|SDL_SWSURFACE;
	}
	if ((screen = SDL_SetVideoMode(320, 240, 32, sdlFlags)) == NULL) {
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		goto fail;
	}

	if (useGL) {
		/* Set up OpenGL viewport and projection. */
		AG_GL_InitContext(AG_RECT(0,0,320,240));
	}

	/* Initialize Agar-GUI to reuse display */
	if (AG_InitVideoSDL(screen, 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		AG_Destroy();
		goto fail;
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);

	/* Display some test widgets. */
	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_LabelNew(win, 0, "Attached to existing %s display",
	    useGL ? "SDL/OpenGL" : "SDL");

	/* Test conversion from SDL_Surface to AG_Surface. */
	if ((bmp = SDL_LoadBMP("agar.bmp")) != NULL) {
		if ((agbmp = AG_SurfaceFromSDL(bmp)) != NULL) {
			AG_PixmapFromSurface(win, 0, agbmp);
		} else {
			AG_LabelNewS(win, 0, AG_GetError());
		}
	} else {
		AG_LabelNewS(win, 0, SDL_GetError());
	}

	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();

	SDL_Quit();
	return (0);
fail:
	SDL_Quit();
	return (1);
#else
	fprintf(stderr, "Agar was compiled without SDL support\n");
	return (1);
#endif /* HAVE_SDL */
}
