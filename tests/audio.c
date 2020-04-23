/*	Public domain	*/
/*
 * Test for the Agar audio library.
 */

#include "config/have_agar_au.h"
#if defined(HAVE_AGAR_AU) && !defined(_WIN32)

#include "agartest.h"

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

static int
TestGUI(void *obj, AG_Window *win)
{
	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);

	AG_ButtonNewFn(win, 0, "Play with PortAudio", OpenOut, "%s", "pa");
	AG_ButtonNewFn(win, 0, "Render to out.wav", OpenOut, "%s", "file(out.wav)");

	AG_SeparatorNewHoriz(win);

	AG_ButtonNewFn(win, 0, "Close output", CloseOut, NULL);
	return (0);
}

const AG_TestCase audioTest = {
	"audio",
	"Test the Agar audio library",
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* test */
	TestGUI,
	NULL			/* bench */
};

#endif /* HAVE_AGAR_AU && !_WIN32 */
