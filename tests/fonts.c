/*	Public domain	*/

#include "agartest.h"

static AG_Font *myFont;

static void
SelectedFont(AG_Event *event)
{
/*	AG_Window *win = AG_WINDOW_PTR(1); */

	if (myFont) {
		AG_SetDefaultFont(myFont);
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
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_FontSelector *fs;
	AG_Box *box;
	int i;

	myFont = agDefaultFont;

	for (i = 0; i <= 10; i++) {
		TestMsg(obj,
		    "Core Font #%d ("
		    AGSI_FONT9 "AGSI_FONT%d" AGSI_RST
		    "): \x1b[%dm%s\x1b[0m",
		    i+1, i+1, 10+i, agCoreFonts[i]);
	}

	fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);
	AG_BindPointer(fs, "font", (void *)&myFont);

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(box, 0, _("Set as Default Font"), SelectedFont, "%p", win);
		AG_ButtonNewFn(box, 0, _("Cancel"), AG_WindowCloseGenEv, "%p", win);
	}
	return (0);
}

#if 0
/*
 * Unsafe microbenchmark (calls AG_TextRender() and AG_TextSize() outside
 * of rendering context).
 */
static void
TextSize_UTF8(void *obj)
{
	int w, h;
	int i;

	for (i = 0; i < 50; i++)
		AG_TextSize("The Quick Brown Fox Jumps Over The Lazy Dog", &w, &h);
}

static void
TextSize_Nat(void *obj)
{
	AG_Char text[] = { 'T','h','e',' ','Q','u','i','c','k',' ',
	                   'B','r','o','w','n',' ','F','o','x',' ',
	                   'J','u','m','p','s',' ','O','v','e','r',' ',
	                   'T','h','e',' ','L','a','z','y',' ',
	                   'D','o','g', '\0' };
	int w, h;
	int i;

	for (i = 0; i < 50; i++)
		AG_TextSizeInternal(text, &w, &h);
}

static void
TextRender_UTF8(void *obj)
{
	AG_Surface *S;

	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog");
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
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Surface *S;

	S = AG_TextRenderInternal(text, ts->font, &ts->colorBG, &ts->color);
	AG_SurfaceFree(S);
}

static void
TextRender_2L(void *obj)
{
	AG_Surface *S;

	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n");
	AG_SurfaceFree(S);
}

static void
TextRender_3L(void *obj)
{
	AG_Surface *S;

	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n");
	AG_SurfaceFree(S);
}

static void
TextRender_4L(void *obj)
{
	AG_Surface *S;

	S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n"
	                  "The Quick Brown Fox Jumps Over The Lazy Dog\n");
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
#endif

const AG_TestCase fontsTest = {
	"fonts",
	N_("Test font engine and AG_FontSelector(3)"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* test */
	TestGUI,
	NULL,			/* bench */
};
