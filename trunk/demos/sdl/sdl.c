/*	Public domain	*/
/*
 * Test functionality specific to SDL, notably attaching to an existing
 * SDL display surface, and converting a SDL_Surface to an AG_Surface.
 */

#include <agar/config/have_sdl.h>
#include <agar/config/have_opengl.h>
#include "config/have_sdl_image.h"

#ifdef HAVE_SDL
#include <SDL.h>
# ifdef HAVE_SDL_IMAGE
#  include <SDL_image.h>
# endif
#endif

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>

int
main(int argc, char *argv[])
{
#ifdef HAVE_SDL
	AG_Window *win;
	SDL_Surface *screen, *bmp, *tex1, *tex2, *avatar;
	AG_Surface *agbmp, *agavatar;
	int c, useGL = 0;
	Uint32 sdlFlags = 0;
	char *optArg;
	Uint32 rmask, gmask, bmask, amask;

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
	}
#ifdef HAVE_SDL_IMAGE
	if ((tex1 = IMG_Load("test1.png")) == NULL ||
	    (tex2 = IMG_Load("test2.png")) == NULL) {
		fprintf(stderr, "IMG_Load() failed\n");
		goto fail;
	}
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
# else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
# endif
	avatar = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA, 64, 128,
                tex1->format->BitsPerPixel, rmask, gmask, bmask, amask);
	SDL_SetAlpha(tex1, 0, 0);
        SDL_BlitSurface(tex1, NULL, avatar, NULL);
	SDL_SetAlpha(tex2, SDL_SRCALPHA, 0);
        SDL_BlitSurface(tex2, NULL, avatar, NULL);
	if ((agavatar = AG_SurfaceFromSDL(avatar)) != NULL) {
		AG_PixmapFromSurface(win, 0, agavatar);
	} else {
		AG_LabelNewS(win, 0, AG_GetError());
	}
#endif /* HAVE_SDL_IMAGE */

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
