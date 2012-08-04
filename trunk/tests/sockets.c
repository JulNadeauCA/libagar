/*	Public domain	*/
/*
 * This program demonstrates the use of drag-and-drop with AG_Icon and
 * AG_Socket widgets in a fixed container.
 */

#include "agartest.h"

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

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Fixed *fx;
	AG_Label *lbl;
	AG_Socket *sock;
	AG_Pixmap *px;
	AG_Icon *helmet, *sword, *axe;
	int i;

	/* Create a fixed widget container */
	fx = AG_FixedNew(win, AG_FIXED_EXPAND);

	/* Create a pixmap */
	if ((px = AG_PixmapFromBMP(fx, 0, "Images/menubg.bmp")) == NULL) {
		TestMsg(obj, "Cannot find menubg.bmp: %s", AG_GetError());
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

	AG_WindowSetPadding(win, 0, 0, 0, 0);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_BC, 640, 128);
	agColors[WINDOW_BG_COLOR] = AG_ColorRGB(0,0,0);
	return (0);
}

const AG_TestCase socketsTest = {
	"sockets",
	N_("Test drag-and-drop with AG_Icon(3) and AG_Socket(3)"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI
};
