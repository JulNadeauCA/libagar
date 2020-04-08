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
	AXE,
	LAST_IMAGE
};
const char *imageFiles[] = {
	"helmet-socket",
	"sword-socket",
	"helmet",
	"sword",
	"axe"
};
AG_Surface *pixmaps[LAST_IMAGE];

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
		AG_TextTmsg(AG_MSG_ERROR, 1000,
		    _("%s is a %s (not a helmet)"),
		    AGOBJECT(icon)->name, itemType);
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
		AG_TextTmsg(AG_MSG_ERROR, 1000,
		    _("%s is a %s (not a weapon)"),
		    AGOBJECT(icon)->name, itemType);
	}
	return (1);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_Fixed *fx;
	AG_Label *lbl;
	AG_Socket *sock;
	AG_Pixmap *px;
	AG_Icon *helmet, *sword, *axe;
	int i;

	/* Create a fixed widget container */
	fx = AG_FixedNew(win, AG_FIXED_EXPAND);

	/* Create a pixmap */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "menubg.bmp", path, sizeof(path))) {
		if ((px = AG_PixmapFromFile(fx, 0, path)) == NULL) {
			TestMsg(obj, "%s: %s", path, AG_GetError());
			return (-1);
		}
		AG_FixedMove(fx, px, 0, 0);
	}
	
	lbl = AG_LabelNew(NULL, 0, AGSI_LEAGUE_SPARTAN "Sockets" AGSI_RST);
	AG_SetStyle(lbl, "text-color", "#ccc");
	AG_SetStyle(lbl, "font-size", "200%");
	AG_FixedPut(fx, lbl, 20, 32);

	lbl = AG_LabelNew(NULL, 0, AGSI_LEAGUE_GOTHIC "( in an AG_Fixed )" AGSI_RST);
	AG_SetStyle(lbl, "text-color", "#aaa");
	AG_SetStyle(lbl, "font-size", "120%");
	AG_FixedPut(fx, lbl, 20, 64);

	/* Load some pixmaps */
	for (i = 0; i < LAST_IMAGE; i++) {
		char bmpFile[AG_FILENAME_MAX];

		Strlcpy(bmpFile, imageFiles[i], sizeof(bmpFile));
		Strlcat(bmpFile, ".bmp", sizeof(bmpFile));
		if (AG_ConfigFind(AG_CONFIG_PATH_DATA, bmpFile,
		    path, sizeof(path)) != 0) {
			continue;
		}
		pixmaps[i] = AG_SurfaceFromFile(path);
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
	 * Create some drag-and-droppable icons.
	 */

	helmet = AG_IconFromSurface(pixmaps[HELMET]);
	AG_SetString(helmet, "item-type", "helmet");
	AG_IconSetTextS(helmet, AGSI_LEAGUE_GOTHIC "Helmet" AGSI_RST);

	sword = AG_IconFromSurface(pixmaps[SWORD]);
	AG_SetString(sword, "item-type", "weapon");
	AG_IconSetTextS(sword, AGSI_LEAGUE_GOTHIC "Sword" AGSI_RST);

	axe = AG_IconFromSurface(pixmaps[AXE]);
	AG_SetString(axe, "item-type", "weapon");
	AG_IconSetTextS(axe, AGSI_LEAGUE_GOTHIC "Axe" AGSI_RST);

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

	AG_SetStyle(win, "padding", "0");
	AG_WindowSetGeometryAligned(win, AG_WINDOW_BC, 642, 200);
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
	TestGUI,
	NULL		/* bench */
};
