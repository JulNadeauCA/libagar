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
			AG_TextTmsg(AG_MSG_INFO, 1000, "Default font has changed.");
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

	TestMsg(obj,
	    "Al-arabiyyah: "
	    AGSI_UNI
	    "\xD9\x8F\xD8\xA9\xD9\x91\xD9\x8E"
	    "\xD9\x8A\xD9\x90\xD8\xA8\xD9\x8E"
	    "\xD8\xB1\xD9\x8E\xD8\xB9\xD9\x92"
	    "\xD9\x84\xD9\x8E\xD8\xA7"
	    AGSI_RST);

	TestMsg(obj,
	    "Devanagari: "
	    AGSI_UNI
	    "\xE0\xA4\xA6 " /* U+0926 DA */
	    "\xE0\xA4\xB5 " /* U+0926 VA */
	    "\xE0\xA4\xA8 " /* U+0928 NA */
	    "\xE0\xA4\x97 " /* U+0917 GA */
	    "\xE0\xA4\xB0 " /* U+0917 RA */
	    AGSI_RST);

	TestMsg(obj,
	    "Ellinik\xC3\xA1: "
	    AGSI_UNI
	    "\xCE\xB5"
	    "\xCE\xBB"
	    "\xCE\xBB"
	    "\xCE\xB7"
	    "\xCE\xBD"
	    "\xCE\xB9"
	    "\xCE\xBA"
	    "\xCE\xAC"
	    AGSI_RST);

	TestMsg(obj,
	    "Gu\xC4\x81nhu\xC3\xA0: "
	    AGSI_CJK
	    "\xE5\xAE\x98" /* U+5b98 */
	    "\xE8\xAF\x9D" /* U+8bdd */
	    AGSI_RST);

	TestMsg(obj,
	    "Hanguk-eo: "
	    AGSI_CJK
	    "\xED\x95\x9C"
	    "\xEA\xB5\xAD"
	    "\xEC\x96\xB4"
	    AGSI_RST);

	TestMsg(obj,
	    "Ivrit: "
	    AGSI_UNI
	    "\xD7\xAA"
	    "\xD7\x99"
	    "\xD7\xA8"
	    "\xD7\x91"
	    "\xD7\xA2"
	    AGSI_RST);

	TestMsg(obj,
	    "Inuktitut: "
	    AGSI_UNI
	    "\xE1\x90\x83"
	    "\xE1\x93\x84"
	    "\xE1\x92\x83"
	    "\xE1\x91\x8E"
	    "\xE1\x91\x90"
	    "\xE1\x91\xA6"
	    AGSI_RST);

	TestMsg(obj,
	    "Kartuli: "
	    AGSI_UNI
	    "\xE1\x83\xA5"
	    "\xE1\x83\x90"
	    "\xE1\x83\xA0"
	    "\xE1\x83\x97"
	    "\xE1\x83\xA3"
	    "\xE1\x83\x9A"
	    "\xE1\x83\x98"
	    AGSI_RST);

	TestMsg(obj,
	    "Nihongo: "
	    AGSI_CJK
	    "\xE6\x97\xA5"
	    "\xE6\x9C\xAC"
	    "\xE8\xAA\x9E"
	    AGSI_RST);

	TestMsg(obj,
	    "Phasa Lao: "
	    AGSI_UNI
	    "\xE0\xBA\x9E"
	    "\xE0\xBA\xB2"
	    "\xE0\xBA\xAA"
	    "\xE0\xBA\xB2"
	    "\xE0\xBA\xA5"
	    "\xE0\xBA\xB2"
	    "\xE0\xBA\xA7"
	    AGSI_RST);

	TestMsg(obj,
	    "Phasa Thai: "
	    AGSI_UNI
	    "\xE0\xB8\xA0"
	    "\xE0\xB8\xB2"
	    "\xE0\xB8\xA9"
	    "\xE0\xB8\xB2"
	    "\xE0\xB9\x84"
	    "\xE0\xB8\x97"
	    "\xE0\xB8\xA2"
	    AGSI_RST);

	TestMsg(obj,
	    "Russkiy: "
	    AGSI_UNI
	    "\xD1\x80"
	    "\xD1\x83"
	    "\xD1\x81"
	    "\xD1\x81"
	    "\xD0\xBA"
	    "\xD0\xB8"
	    "\xD0\xB9"
	    AGSI_RST);

	TestMsg(obj,
	    "Tibetan digits: "
	    AGSI_UNI
	    "\xE0\xBC\xA0 "
	    "\xE0\xBC\xA1 "
	    "\xE0\xBC\xA2 "
	    "\xE0\xBC\xA3 "
	    "\xE0\xBC\xA4 "
	    "\xE0\xBC\xA5 "
	    "\xE0\xBC\xA6 "
	    "\xE0\xBC\xA7 "
	    "\xE0\xBC\xA8 "
	    AGSI_RST);

	TestMsg(obj, "");

	for (i = 0; i <= 10; i++) {
		TestMsg(obj,
		    "Core Font #%d (AGSI_FONT%d): \x1b[%dm%s\x1b[0m",
		    i+1, i+1, 10+i, agCoreFonts[i]);
	}

	fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);
	AG_BindPointer(fs, "font", (void *)&myFont);

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(box, 0, _("Set as Default Font"), SelectedFont, "%p", win);
		AG_ButtonNewFn(box, 0, _("Cancel"), AGWINCLOSE(win));
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
