/*	Public domain	*/
/*
 * Test the ability of AG_Editable(3) to bind to string buffers in
 * different character set encodings.
 */

#include "agartest.h"

#include <string.h>

#include <agar/config/have_iconv.h>

char myASCII[20*4];
char myUTF[30*4];
char myLat1[30*4];

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Text *myTxt;
	AG_Textbox *tb;

	/* Bind to a C string in US-ASCII */
	AG_Strlcpy(myASCII, "ASCII!", sizeof(myASCII));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL,
	    "ASCII Buffer: \n(%lu bytes)", sizeof(myASCII));
	AG_TextboxBindASCII(tb, myASCII, sizeof(myASCII));
	AG_TextboxSizeHintLines(tb, 2);

	/* Bind to a C string in UTF-8 */
	AG_Strlcpy(myUTF, "\xc3\x85ngstrom!", sizeof(myUTF));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL,
	    "UTF-8 Buffer: \n(%lu bytes)", sizeof(myUTF));
	AG_TextboxBindUTF8(tb, myUTF, sizeof(myUTF));
	AG_TextboxSizeHintLines(tb, 2);

	/* Bind to a C string in any iconv-supported encoding */
#ifdef HAVE_ICONV
	AG_Strlcpy(myLat1, "Overv\xE5knign for feils\xF8king!", sizeof(myLat1));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL,
	    "LATIN-1 Buffer: \n(%lu bytes)", sizeof(myLat1));
	AG_TextboxBindEncoded(tb, "ISO-8859-1", myLat1, sizeof(myLat1));
	AG_TextboxSizeHintLines(tb, 2);
	AG_LabelNewS(win, 0, "iconv support: YES");
#else
	AG_LabelNewS(win, 0,
	    "iconv support: NO.\n"
	    "Available encodings: ASCII, UTF-8, UCS-4");
#endif
	AG_SeparatorNewHoriz(win);

	/* Bind to a multilingual AG_Text(3) element. */
	myTxt = AG_TextNew(0);
	AG_TextSetEnt(myTxt, AG_LANG_FR, "Fran\xc3\xa7\x61is!");
	AG_TextSetEnt(myTxt, AG_LANG_EN, "English!");
	AG_TextSetEnt(myTxt, AG_LANG_NO, "Norsk!");
	AG_LabelNewS(win, 0, "Multilingual AG_Text(3) buffer:");
	tb = AG_TextboxNewS(win,
	    AG_TEXTBOX_MULTILINGUAL|AG_TEXTBOX_EXPAND|AG_TEXTBOX_MULTILINE,
	    NULL);
	AG_TextboxBindText(tb, myTxt);
	AG_TextboxSetLang(tb, AG_LANG_FR);
	AG_TextboxSizeHint(tb, "XXXXXXXXXXXXXXXXXXXXXXXXX");
	AG_TextboxSizeHintLines(tb, 5);
	AG_TextboxSetCursorPos(tb, -1);		/* End of string */
	return (0);
}

const AG_TestCase charsetsTest = {
	"charsets",
	N_("Test AG_Editable(3) bound to buffers in different character sets"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
