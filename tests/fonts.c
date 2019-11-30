/*	Public domain	*/

#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	AG_Mutex lock;
} MyTestInstance;

static void
SelectedFont(AG_Event *event)
{
/*	AG_Window *win = AG_WINDOW_PTR(1); */

	AG_SetString(agConfig, "font.face", AGOBJECT(agDefaultFont)->name);
	AG_SetInt(agConfig, "font.size", agDefaultFont->spec.size);
	AG_SetUint(agConfig, "font.flags", agDefaultFont->flags);
	if (AG_ConfigSave() == 0) {
#ifdef AG_TIMERS
		AG_TextTmsg(AG_MSG_INFO, 1000, "Default font has changed.");
#endif
	} else {
		AG_TextMsgFromError();
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_FontSelector *fs;
	AG_Box *hBox;
	
	fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);
	AG_BindPointer(fs, "font", (void *)&agDefaultFont);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(hBox, 0, _("OK"), SelectedFont, "%p", win);
		AG_ButtonNewFn(hBox, 0, _("Cancel"), AG_WindowCloseGenEv, "%p", win);
	}
	return (0);
}

static void
TextSize_UTF8(void *obj)
{
#ifdef AG_THREADS
	MyTestInstance *ti = obj;
#endif
	int w, h;
	int i;

	AG_MutexLock(&ti->lock);
	for (i = 0; i < 50; i++) {
		AG_TextSize("The Quick Brown Fox Jumps Over The Lazy Dog", &w, &h);
	}
	AG_MutexUnlock(&ti->lock);
}

static void
TextSize_Nat(void *obj)
{
	AG_Char text[] = { 'T','h','e',' ','Q','u','i','c','k',' ',
	                   'B','r','o','w','n',' ','F','o','x',' ',
	                   'J','u','m','p','s',' ','O','v','e','r',' ',
	                   'T','h','e',' ','L','a','z','y',' ',
	                   'D','o','g', '\0' };
#ifdef AG_THREADS
	MyTestInstance *ti = obj;
#endif
	int w, h;
	int i;

	AG_MutexLock(&ti->lock);
	for (i = 0; i < 50; i++) {
		AG_TextSizeInternal(text, &w, &h);
	}
	AG_MutexUnlock(&ti->lock);
}

static void
TextRender_UTF8(void *obj)
{
#ifdef AG_THREADS
	MyTestInstance *ti = obj;
#endif
	AG_Surface *S;

	AG_MutexLock(&ti->lock);
	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog");
	AG_MutexUnlock(&ti->lock);
	AG_SurfaceFree(S);
}

static void
TextRender_Nat(void *obj)
{
	AG_Char text[] = { 'T','h','e',' ','Q','u','i','c','k',' ',
	                   'B','r','o','w','n',' ','F','o','x',' ',
	                   'J','u','m','p','s',' ','O','v','e','r',' ',
	                   'T','h','e',' ','L','a','z','y',' ','D','o','g',
	                   '\0' };
#ifdef AG_THREADS
	MyTestInstance *ti = obj;
#endif
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Surface *S;

	AG_MutexLock(&ti->lock);
	S = AG_TextRenderInternal(text, ts->font, &ts->colorBG, &ts->color);
	AG_MutexUnlock(&ti->lock);
	AG_SurfaceFree(S);
}

static void
TextRender_2L(void *obj)
{
#ifdef AG_THREADS
	MyTestInstance *ti = obj;
#endif
	AG_Surface *S;

	AG_MutexLock(&ti->lock);
	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n");
	AG_MutexUnlock(&ti->lock);
	AG_SurfaceFree(S);
}

static void
TextRender_3L(void *obj)
{
#ifdef AG_THREADS
	MyTestInstance *ti = obj;
#endif
	AG_Surface *S;

	AG_MutexLock(&ti->lock);
	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n");
	AG_MutexUnlock(&ti->lock);
	AG_SurfaceFree(S);
}

static void
TextRender_4L(void *obj)
{
#ifdef AG_THREADS
	MyTestInstance *ti = obj;
#endif
	AG_Surface *S;

	AG_MutexLock(&ti->lock);
	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n");
	AG_MutexUnlock(&ti->lock);
	AG_SurfaceFree(S);
}

static struct ag_benchmark_fn fontBenchFns[] = {
	{ "TextSize_Nat",	TextSize_Nat },
	{ "TextSize_UTF8",	TextSize_UTF8 },
	{ "TextRender_Nat",	TextRender_Nat },
	{ "TextRender_UTF8",	TextRender_UTF8 },
	{ "TextRender_2L",	TextRender_2L },
	{ "TextRender_3L",	TextRender_3L },
	{ "TextRender_4L",	TextRender_4L },
};
static struct ag_benchmark fontBench = {
	"Fonts",
	&fontBenchFns[0],
	sizeof(fontBenchFns) / sizeof(fontBenchFns[0]),
	10, 500, 2000000000
};

static int
Bench(void *obj)
{
	AG_TestInstance *ti = obj;

	TestMsgS(ti, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	TestMsgS(ti, "abcdefghijklmnopqrstuvwxyz");
	TestMsgS(ti, "0123456789-=.,;`[]'~!@#$%^&*()_+\\/");
	TestExecBenchmark(obj, &fontBench);
	return (0);
}

static int
Init(void *obj)
{
#ifdef AG_THREADS
	MyTestInstance *ti = obj;
	AG_MutexInit(&ti->lock);
#endif
	return (0);
}

const AG_TestCase fontsTest = {
	"fonts",
	N_("Test font engine and AG_FontSelector(3)"),
	"1.6.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	Bench
};
