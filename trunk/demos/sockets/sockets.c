/*	Public domain	*/
/*
 * This program demonstrates the use of drag-and-drop with AG_Icon and
 * AG_Socket widgets in a fixed container.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

enum {
	HELMET_SOCKET,
	WEAPON_SOCKET,
	HELMET,
	SWORD,
	AXE
};
AG_Surface *pixmaps[4];

/* Callback for helmet sockets */
static int
InsertHelmet(AG_Socket *sock, AG_Icon *icon)
{
	char itemType[64];

	AG_GetString(icon, "item-type", itemType, sizeof(itemType));
	if (strcmp(itemType, "helmet") == 0) {
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
	char itemType[64];

	AG_GetString(icon, "item-type", itemType, sizeof(itemType));
	if (strcmp(itemType, "weapon") == 0) {
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
	unsigned int wView, hView;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 100, 70);
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

	/*
	 * Create a fixed-size window with a Fixed container.
	 */
	win = AG_WindowNewNamedS(AG_WINDOW_PLAIN, "game-menu");
	AG_WindowSetPadding(win, 0, 0, 0, 0);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_BL, 100, 30);
	agColors[WINDOW_BG_COLOR] = AG_ColorRGB(0,0,0);

	fx = AG_FixedNew(win, AG_FIXED_EXPAND);

	if ((px = AG_PixmapFromBMP(fx, 0, "Images/menubg.bmp")) == NULL) {
		fprintf(stderr, "Cannot find menubg.bmp\n", AG_GetError());
		exit(1);
	}
	AG_FixedMove(fx, px, 0, 0);
	
	lbl = AG_LabelNew(NULL, 0, "Drag & Drop Demo");
	AG_FixedPut(fx, lbl, 20, 32);

	/* Load some pixmaps */
	pixmaps[HELMET_SOCKET] = AG_SurfaceFromBMP("Images/helmet-socket.bmp");
	pixmaps[WEAPON_SOCKET] = AG_SurfaceFromBMP("Images/sword-socket.bmp");
	pixmaps[HELMET] = AG_SurfaceFromBMP("Images/helmet.bmp");
	pixmaps[SWORD] = AG_SurfaceFromBMP("Images/sword.bmp");
	pixmaps[AXE] = AG_SurfaceFromBMP("Images/axe.bmp");
	for (i = 0; i < 5; i++) {
		AG_SurfaceSetColorKey(pixmaps[i], AG_SRCCOLORKEY,
		    AG_MapPixelRGB(pixmaps[i]->format, 0,255,0));
	}

	/*
	 * Create some empty sockets.
	 */

	sock = AG_SocketFromSurface(fx, 0, pixmaps[HELMET_SOCKET]);
	AG_SocketInsertFn(sock, InsertHelmet);
	AG_FixedMove(fx, sock, 200, 32);
	AG_FixedSize(fx, sock, 32, 32);

	sock = AG_SocketFromSurface(fx, 0, pixmaps[WEAPON_SOCKET]);
	AG_SocketInsertFn(sock, InsertWeapon);
	AG_FixedMove(fx, sock, 242, 32);
	AG_FixedSize(fx, sock, 32, 32);

	/*
	 * Create some icons.
	 */

	helmet = AG_IconFromSurface(pixmaps[HELMET]);
	AG_SetString(helmet, "item-type", "helmet");
	sword = AG_IconFromSurface(pixmaps[SWORD]);
	AG_SetString(sword, "item-type", "weapon");
	axe = AG_IconFromSurface(pixmaps[AXE]);
	AG_SetString(axe, "item-type", "weapon");

	/*
	 * Create some populated sockets.
	 */

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
	char *driverSpec = "<SDL>", *optArg;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: sockets [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-sockets-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	CreateGameView();
	CreateGameMenu();

	AG_EventLoop();
	AG_Destroy();
	return (0);
}
