/*	Public domain	*/
/*
 * This program tests Chinese text rendering.
 */

#include "agartest.h"

#define TESTFONT "HAN NOM B.ttf"

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Label *lbl;
	AG_Font *font;

	if (AG_FileExists(TESTFONT) &&
	    (font = AG_FetchFont(TESTFONT, 18, 0)) != NULL) {
		lbl = AG_LabelNew(win, AG_LABEL_EXPAND,
		    "\xE6\xB1\x89\xE8\xAF\xAD\x2F"
		    "\xE6\xBC\xA2\xE8\xAA\x9E\x0A");
		AG_LabelSetFont(lbl, font);
	} else {
		AG_SetError(
		    "This demo requires the font \"%s\","
		    "please download it from:\n"
		    "http://vietunicode.sourceforge.net/fonts/fonts_hannom.html\n"
		    "and place it in the current directory.", TESTFONT);
		AG_TextMsgFromError();
		return (-1);
	}
	return (0);
}

const AG_TestCase chineseTest = {
	"chinese",
	N_("Test Chinese text rendering"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI
};
