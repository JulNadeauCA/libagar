/*	Public domain	*/
/*
 * Test for the Agar audio library.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/au.h>

#include <string.h>
#include <math.h>

AU_DevOut *auOut = NULL;
AG_Thread outTh;
AG_Mutex outLock;
int outAbort = 0;
AG_Console *cons;

static void CloseOut(AG_Event *);

static void *
GenSine(void *p)
{
	const Uint nCh = 2;
	const Uint bufSamples = 4000;
	const Uint bufSize = bufSamples*nCh;
	float x, buf[bufSize];
	int i, ch;
	double k;
	
	x = 0.0;
	for (;;) {
		AG_MutexLock(&outLock);
		if (outAbort) {
			outAbort = 0;
			AG_MutexUnlock(&outLock);
			break;
		}
		for (i = 0; i < bufSamples; i++) {
			for (ch = 0; ch < nCh; ch++)
				buf[i+ch] = sin(x)/2.0;
		}
		if (AU_WriteFloat(auOut, buf, bufSamples)) {
			AG_ConsoleMsg(cons, "Write error (AU_WriteFloat)");
		}
		AG_MutexUnlock(&outLock);

	}
	return (NULL);
}

static void
OpenOut(AG_Event *event)
{
	char *device = AG_STRING(1);

	if (auOut != NULL) {
		return;
	}
	if ((auOut = AU_OpenOut(device, 44100, 2)) == NULL) {
		AG_ConsoleMsg(cons, "Failed: %s", AG_GetError());
		return;
	}
	AG_ConsoleMsg(cons, "Writing to %s at %dHz / %dCh",
	    device, auOut->rate, auOut->ch);
	AG_ThreadCreate(&outTh, GenSine, NULL);
}

static void
CloseOut(AG_Event *event)
{
	if (auOut == NULL) {
		return;
	}
	outAbort = 1;
	for (;;) {
		if (!outAbort) {
			break;
		}
		AG_Delay(100);
	}

	if (auOut != NULL) {
		AU_CloseOut(auOut);
		auOut = NULL;
	}
	AG_ConsoleMsg(cons, "Closed device OK");
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	char *driverSpec = NULL, *optArg;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: agaraudio [-d agar-driver-spec]\n");
			return (1);
		}
	}

	if (AG_InitCore(NULL, 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1 ||
	    AU_InitSubsystem() == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindStdGlobalKeys();
	AG_MutexInit(&outLock);

	win = AG_WindowNew(AG_WINDOW_MAIN);
	AG_WindowSetCaption(win, "Agar audio test");
	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	AG_ButtonNewFn(win, 0, "Play with PortAudio",
	    OpenOut, "%s", "pa");
	AG_ButtonNewFn(win, 0, "Render to out.wav",
	    OpenOut, "%s", "file(out.wav)");
	AG_SeparatorNewHoriz(win);
	AG_ButtonNewFn(win, 0, "Close output", CloseOut, NULL);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 200);
	AG_WindowShow(win);

	AU_DestroySubsystem();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
