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
#define SINEBUF 4000
	float x, out[SINEBUF*2];
	int i, j;
	double df = 0.010;
	double k;
	
	x = 0.0;
	for (;;) {
		AG_MutexLock(&outLock);
		if (outAbort) {
			outAbort = 0;
			AG_MutexUnlock(&outLock);
			break;
		}
		for (i = 0; i < SINEBUF*2; i++) {
			out[i] = sin(x)/2.0;
			x += df;
		}
		for (k = 0.0, i = 0;
		     k < 1.0 && i < 2000;
		     k+=0.0005, i++) {
			out[i] = out[i]*k;
			out[SINEBUF*2-i] = out[SINEBUF*2-i]*k;
		}
		if (AU_WriteFloat(auOut, out, SINEBUF)) {
			AG_ConsoleMsg(cons, "Write error (AU_WriteFloat)");
		}
		AG_MutexUnlock(&outLock);
		AG_Delay(200);
		df += 0.0001;
	}
	return (NULL);
}

static void
OpenFileOut(AG_Event *event)
{
	if (auOut != NULL) {
		return;
	}
	if ((auOut = AU_OpenOut("file(foo.wav)", 44100, 2)) == NULL) {
		AG_ConsoleMsg(cons, "Failed: %s", AG_GetError());
		return;
	}
	AG_ConsoleMsg(cons, "Writing to foo.wav at %dHz / %dCh",
	    auOut->rate, auOut->ch);
	AG_ThreadCreate(&outTh, GenSine, NULL);
}

static void
OpenAudioOut(AG_Event *event)
{
	if (auOut != NULL) {
		CloseOut(NULL);
	}
	if ((auOut = AU_OpenOut("pa", 44100, 2)) == NULL) {
		AG_ConsoleMsg(cons, "Failed: %s", AG_GetError());
		return;
	}
	AG_ConsoleMsg(cons, "Writing to PortAudio2 at %dHz / %dCh",
	    auOut->rate, auOut->ch);
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

static int
Init(void *obj)
{
	return AU_InitSubsystem();
}

static void
Destroy(void *obj)
{
	AU_DestroySubsystem();
}

static int
TestGUI(void *obj, AG_Window *win)
{
	return (0);
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
#ifdef __APPLE__
	AG_BindGlobalKey(AG_KEY_Q, AG_KEYMOD_META, AG_QuitGUI);
#else
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
#endif
	
	AG_MutexInit(&outLock);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar audio test");
	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	AG_ButtonNewFn(win, 0, "Test sndfile output", OpenFileOut, NULL);
	AG_ButtonNewFn(win, 0, "Test portaudio output ", OpenAudioOut, NULL);
	AG_ButtonNewFn(win, 0, "Close output", CloseOut, NULL);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 200);
	AG_WindowShow(win);

	AU_DestroySubsystem();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
