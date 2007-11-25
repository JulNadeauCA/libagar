/*	Public domain	*/
/*
 * This program demonstrates the use of drag-and-drop with AG_Icon and
 * AG_Socket widgets in a fixed container.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

enum {
	HELMET_SOCKET,
	WEAPON_SOCKET,
	HELMET,
	SWORD,
	AXE
};
SDL_Surface *pixmaps[4];

/* Callback for helmet sockets */
static int
InsertHelmet(AG_Socket *sock, AG_Icon *icon)
{
	if (strcmp(AG_String(icon,"item-type"), "helmet") == 0) {
		if (icon->sock != NULL) {
			AG_SocketRemoveIcon(icon->sock);
		}
		AG_SocketInsertIcon(sock, icon);
	} else {
		printf("Not a helmet!\n");
	}
	return (1);
}

/* Callback for weapon sockets */
static int
InsertWeapon(AG_Socket *sock, AG_Icon *icon)
{
	if (strcmp(AG_String(icon,"item-type"), "weapon") == 0) {
		if (icon->sock != NULL) {
			AG_SocketRemoveIcon(icon->sock);
		}
		AG_SocketInsertIcon(sock, icon);
	} else {
		printf("Not a weapon!\n");
	}
	return (1);
}

static void
CreateGameView(void)
{
	AG_Window *win;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_WindowSetGeometry(win,
	    0,		0,
	    agView->w,	agView->h-128);
	AG_WindowShow(win);
}

static void
CreateGameMenu(void)
{
	AG_Window *win;
	AG_Fixed *fx;
	AG_Label *lbl;
	AG_Socket *sock;
	AG_Pixmap *px;
	AG_Icon *helmet, *sword, *axe;
	int i;

	/* Create a fixed-size window with a Fixed container. */
	win = AG_WindowNewNamed(AG_WINDOW_PLAIN, "game-menu");
	AG_WindowSetPadding(win, 0, 0, 0, 0);
	AG_WindowSetGeometry(win,
	    0,		agView->h-128,
	    agView->w,	128);
	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 0, 0, 0);

	fx = AG_FixedNew(win, AG_FIXED_EXPAND);
	if ((px = AG_PixmapFromBMP(fx, 0, "menubg.bmp")) == NULL) {
		fprintf(stderr, "Cannot find menubg.bmp\n", AG_GetError());
		exit(1);
	}
	AG_FixedMove(fx, px, 0, 0);
	
	lbl = AG_LabelNewStatic(NULL, 0, "Drag & Drop Demo (%s)",
	    agView->opengl ? "OpenGL" : "SDL");
	AG_FixedPut(fx, lbl, 20, 32);

	/* Load some pixmaps */
	pixmaps[HELMET_SOCKET] = SDL_LoadBMP("helmet-socket.bmp");
	pixmaps[WEAPON_SOCKET] = SDL_LoadBMP("sword-socket.bmp");
	pixmaps[HELMET] = SDL_LoadBMP("helmet.bmp");
	pixmaps[SWORD] = SDL_LoadBMP("sword.bmp");
	pixmaps[AXE] = SDL_LoadBMP("axe.bmp");
	for (i = 0; i < 5; i++) {
		SDL_SetColorKey(pixmaps[i], SDL_SRCCOLORKEY,
		    SDL_MapRGB(pixmaps[i]->format, 0, 255, 0));
	}

	/* Create some empty sockets. */
	sock = AG_SocketFromSurface(fx, 0, pixmaps[HELMET_SOCKET]);
	AG_SocketInsertFn(sock, InsertHelmet);
	AG_FixedMove(fx, sock, 200, 32);
	AG_FixedSize(fx, sock, 32, 32);

	sock = AG_SocketFromSurface(fx, 0, pixmaps[WEAPON_SOCKET]);
	AG_SocketInsertFn(sock, InsertWeapon);
	AG_FixedMove(fx, sock, 242, 32);
	AG_FixedSize(fx, sock, 32, 32);

	/* Create some icons. */
	helmet = AG_IconFromSurface(pixmaps[HELMET]);
	AG_SetString(helmet, "item-type", "helmet");
	sword = AG_IconFromSurface(pixmaps[SWORD]);
	AG_SetString(sword, "item-type", "weapon");
	axe = AG_IconFromSurface(pixmaps[AXE]);
	AG_SetString(axe, "item-type", "weapon");

	/* Create some populated sockets. */
	sock = AG_SocketFromSurface(fx, 0, pixmaps[HELMET_SOCKET]);
	AG_SocketInsertFn(sock, InsertHelmet);
	AG_FixedMove(fx, sock, 332, 32);
	AG_FixedSize(fx, sock, 32, 32);
	AG_SocketInsertIcon(sock, helmet);

	sock = AG_SocketFromSurface(fx, 0, pixmaps[WEAPON_SOCKET]);
	AG_SocketInsertFn(sock, InsertWeapon);
	AG_FixedMove(fx, sock, 374, 32);
	AG_FixedSize(fx, sock, 32, 32);
	AG_SocketInsertIcon(sock, sword);
	
	sock = AG_SocketFromSurface(fx, 0, pixmaps[WEAPON_SOCKET]);
	AG_SocketInsertFn(sock, InsertWeapon);
	AG_FixedMove(fx, sock, 416, 32);
	AG_FixedSize(fx, sock, 32, 32);
	AG_SocketInsertIcon(sock, axe);

	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("sockets-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_SetBool(agConfig, "view.opengl", 1);
	while ((c = getopt(argc, argv, "?vfFgGr:")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
		case 'f':
			AG_SetBool(agConfig, "view.full-screen", 1);
			break;
		case 'F':
			AG_SetBool(agConfig, "view.full-screen", 0);
			break;
		case 'G':
			AG_SetBool(agConfig, "view.opengl", 0);
			break;
		case 'r':
			fps = atoi(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfFgG] [-r fps]\n", agProgName);
			exit(0);
		}
	}
	if (AG_InitVideo(640, 480, 32, 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	CreateGameView();
	CreateGameMenu();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}

