/*	Public domain	*/
/*
 * Test for the Agar audio library.
 */

#include "agartest.h"

#include <agar/au.h>

#include <string.h>
#include <math.h>

AU_DevOut *auOut = NULL;
AG_Thread outTh;
AG_Mutex outLock;
int outAbort = 0;

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
			fprintf(stderr, "AU_WriteFloat: failed\n");
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
	if ((auOut = AU_OpenOut("file(foo.wav)", 44100, 2)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	AG_TextTmsg(AG_MSG_INFO, 500, "Opened device OK (%dHz / %dCh)",
	    auOut->rate, auOut->ch);
	AG_ThreadCreate(&outTh, GenSine, NULL);
}

static void
OpenAudioOut(AG_Event *event)
{
	if ((auOut = AU_OpenOut("pa", 44100, 2)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	AG_TextTmsg(AG_MSG_INFO, 500, "Opened device OK (%dHz / %dCh)",
	    auOut->rate, auOut->ch);
	AG_ThreadCreate(&outTh, GenSine, NULL);
}

static void
CloseOut(AG_Event *event)
{
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
	AG_TextTmsg(AG_MSG_INFO, 1000, "Closed device OK");
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
	AG_MutexInit(&outLock);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Open File Out", OpenFileOut, NULL);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Open Audio Out", OpenAudioOut, NULL);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Close Audio Out", CloseOut, NULL);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 200);
	return (0);
}

const AG_TestCase audioTest = {
	"audio",
	N_("Test the Agar audio library (ag_au)"),
	"1.4.2",
	AG_TEST_AUDIO,
	sizeof(AG_TestInstance),
	Init,
	Destroy,
	NULL,		/* test */
	TestGUI
};
