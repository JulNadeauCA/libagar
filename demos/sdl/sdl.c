/*	Public domain	*/
/*
 * Test functionality specific to SDL.
 */

#include <agar/config/have_sdl.h>
#ifdef HAVE_SDL
#include <SDL.h>
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

	/* Set up SDL */
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return (1);
	}
	if ((screen = SDL_SetVideoMode(320, 240, 32, SDL_SWSURFACE|SDL_RESIZABLE)) == NULL) {
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		goto fail;
	}
	if ((bmp = SDL_LoadBMP("agar.bmp")) == NULL) {
		fprintf(stderr, "SDL_LoadBMP: %s\n", SDL_GetError());
		goto fail;
	}
	if ((agbmp = AG_SurfaceFromSDL(bmp)) == NULL) {
		fprintf(stderr, "AG_SurfaceFromSDL: %s\n", SDL_GetError());
		goto fail;
	}

	/* Initialize Agar-Core */
	if (AG_InitCore("agar-sdl-demo", 0) == -1) {
		fprintf(stderr, "AG_InitCore: %s\n", AG_GetError());
		goto fail;
	}

	/* Initialize Agar-GUI to reuse display */
	if (AG_InitVideoSDL(screen, 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		AG_Destroy();
		goto fail;
	}

	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_LabelNew(win, 0, "Attached to existing SDL display");
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
