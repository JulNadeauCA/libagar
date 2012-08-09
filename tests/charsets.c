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
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL, "ASCII Buffer: ");
	AG_TextboxBindASCII(tb, myASCII, sizeof(myASCII));
	
	/* Bind to a C string in UTF-8 */
	AG_Strlcpy(myUTF, "\xc3\x85ngstrom!", sizeof(myUTF));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL, "UTF-8 Buffer: ");
	AG_TextboxBindUTF8(tb, myUTF, sizeof(myUTF));

	/* Bind to a C string in any iconv-supported encoding */
#ifdef HAVE_ICONV
	AG_Strlcpy(myLat1, "Overv\xE5knign for feils\xF8king!", sizeof(myLat1));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL, "LATIN-1 Buffer: ");
	AG_TextboxBindEncoded(tb, "ISO-8859-1", myLat1, sizeof(myLat1));
#else
	AG_LabelNewS(win, 0, "(Agar was not compiled with iconv support)");
#endif

	/* Bind to a multilingual AG_Text(3) element. */
	myTxt = AG_TextNewS(NULL);
	AG_TextSetEnt(myTxt, AG_LANG_FR, "Fran\xc3\xa7\x61is!");
	AG_TextSetEnt(myTxt, AG_LANG_EN, "English!");
	AG_TextSetEnt(myTxt, AG_LANG_NO, "Norsk!");
	AG_TextSetLangISO(myTxt, "fr");
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL, "AG_Text Buffer: ");
	AG_TextboxBindText(tb, myTxt);
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
	TestGUI
};
