/*	Public domain	*/

#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	AG_Mutex lock;
} MyTestInstance;

static void
SelectedFont(AG_Event *event)
{
/*	AG_Window *win = AG_PTR(1); */

	AG_SetString(agConfig, "font.face", AGOBJECT(agDefaultFont)->name);
	AG_SetInt(agConfig, "font.size", agDefaultFont->spec.size);
	AG_SetUint(agConfig, "font.flags", agDefaultFont->flags);
	if (AG_ConfigSave() == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000, "Default font has changed.");
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
	MyTestInstance *ti = obj;
	int w, h;
	int i;

	AG_MutexLock(&ti->lock);
	for (i = 0; i < 50; i++) {
		AG_TextSize("The Quick Brown Fox Jumps Over The Lazy Dog", &w, &h);
	}
	AG_MutexUnlock(&ti->lock);
}

static void
TextSize_UCS4(void *obj)
{
	Uint32 ucs4[] = { 'T','h','e',' ','Q','u','i','c','k',' ',
	                  'B','r','o','w','n',' ','F','o','x',' ',
	                  'J','u','m','p','s',' ','O','v','e','r',' ',
	                  'T','h','e',' ','L','a','z','y',' ',
			  'D','o','g', '\0' };
	MyTestInstance *ti = obj;
	int w, h;
	int i;

	AG_MutexLock(&ti->lock);
	for (i = 0; i < 50; i++) {
		AG_TextSizeUCS4(ucs4, &w, &h);
	}
	AG_MutexUnlock(&ti->lock);
}

static void
TextRender_UTF8(void *obj)
{
	MyTestInstance *ti = obj;
	AG_Surface *S;

	AG_MutexLock(&ti->lock);
	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog");
	AG_MutexUnlock(&ti->lock);
	AG_SurfaceFree(S);
}

static void
TextRender_UCS4(void *obj)
{
	Uint32 ucs4[] = { 'T','h','e',' ','Q','u','i','c','k',' ',
	                  'B','r','o','w','n',' ','F','o','x',' ',
	                  'J','u','m','p','s',' ','O','v','e','r',' ',
	                  'T','h','e',' ','L','a','z','y',' ','D','o','g',
			  '\0' };
	MyTestInstance *ti = obj;
	AG_Surface *S;

	AG_MutexLock(&ti->lock);
	S = AG_TextRenderUCS4(ucs4);
	AG_MutexUnlock(&ti->lock);
	AG_SurfaceFree(S);
}

static void
TextRender_2L(void *obj)
{
	MyTestInstance *ti = obj;
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
	MyTestInstance *ti = obj;
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
	MyTestInstance *ti = obj;
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
	{ "TextSize_UCS4",	TextSize_UCS4 },
	{ "TextSize_UTF8",	TextSize_UTF8 },
	{ "TextRender_UCS4",	TextRender_UCS4 },
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
	MyTestInstance *ti = obj;

	AG_MutexInit(&ti->lock);
	return (0);
}

const AG_TestCase fontsTest = {
	"fonts",
	N_("Test font engine and AG_FontSelector(3)"),
	"1.5.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	Bench
};
