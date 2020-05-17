/*	Public domain	*/
/*
 * This test ensures Agar can be destroyed and re-initialized.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <stdlib.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;
	char *driverSpec = NULL, *optArg;
	int i, c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: agarreinit [-d agar-driver-spec]\n");
			return (1);
		}
	}

	for (i = 0; i < 25; i++) {
		printf("Test %d/25:\n", i);
		printf("\tInitCore()\n");
		if (AG_InitCore(NULL, 0) == -1) {
			printf("AG_InitCore(%d): %s\n", i, AG_GetError());
			exit(1);
		}
		printf("\tInitGraphics()\n");
		if (AG_InitGraphics(driverSpec) == -1) {
			printf("AG_InitGraphics(%d): %s\n", i, AG_GetError());
			exit(1);
		}
		printf("\tCreate Window\n");
		win = AG_WindowNew(0);
		AG_WindowSetCaption(win, "foo");
		AG_WindowShow(win);
		printf("\tDestroyVideo()\n");
		AG_DestroyVideo();
		printf("\tDestroy()\n");
		AG_Destroy();
		AG_Delay(100);
	}
	printf("Test successful\n");
	return (0);
}
