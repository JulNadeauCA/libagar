/*	Public domain	*/
/*
 * Test the ability of AG_Editable(3) to bind to string buffers in
 * different character set encodings.
 */

#include <agar/config/ag_unicode.h>
#ifdef AG_UNICODE

#include "agartest.h"

#include <string.h>

#include <agar/config/have_iconv.h>

typedef struct {
	AG_TestInstance _inherit;
	char myASCII[20*4];		/* Example US-ASCII buffer */
	char myUTF[30*4];		/* Example UTF-8 buffer */
	char myLat1[30*4];		/* Example ISO-8859-1 buffer */
	AG_Text *myTxt;			/* Sample AG_TextElement(3) */
} MyTestInstance;

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->myTxt = AG_TextNew(0);
	AG_TextSetEnt(ti->myTxt, AG_LANG_FR, "(UTF8) Fran\xc3\xa7\x61is!");
	AG_TextSetEnt(ti->myTxt, AG_LANG_EN, "(UTF8) English!");
	AG_TextSetEnt(ti->myTxt, AG_LANG_NO, "(UTF8) Norsk!");
	return (0);
}

static void
Destroy(void *obj)
{
	MyTestInstance *ti = obj;

	AG_TextFree(ti->myTxt);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	AG_Textbox *tb;

	/* Bind to a C string in US-ASCII */
	AG_Strlcpy(ti->myASCII, "(US-ASCII)", sizeof(ti->myASCII));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL,
	    "ASCII Buffer: \n(%lu bytes)", (long unsigned)sizeof(ti->myASCII));
	AG_TextboxBindASCII(tb, ti->myASCII, sizeof(ti->myASCII));
	AG_TextboxSizeHintLines(tb, 2);

	/* Bind to a C string in UTF-8 */
	AG_Strlcpy(ti->myUTF, "(UTF8) \xc3\x85ngstrom!", sizeof(ti->myUTF));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL,
	    "UTF-8 Buffer: \n(%lu bytes)", (long unsigned)sizeof(ti->myUTF));
	AG_TextboxBindUTF8(tb, ti->myUTF, sizeof(ti->myUTF));
	AG_TextboxSizeHintLines(tb, 2);

	/* Bind to a C string in any iconv-supported encoding */
#ifdef HAVE_ICONV
	AG_Strlcpy(ti->myLat1, "(LATIN-1) overv\xE5knign for feils\xF8king!", sizeof(ti->myLat1));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL,
	    "LATIN-1 Buffer: \n(%lu bytes)", (long unsigned)sizeof(ti->myLat1));
	AG_TextboxBindEncoded(tb, "ISO-8859-1", ti->myLat1, sizeof(ti->myLat1));
	AG_TextboxSizeHintLines(tb, 2);
	AG_LabelNewS(win, 0, "iconv support: YES");
#else
	AG_LabelNewS(win, 0,
	    "iconv support: NO.\n"
	    "Available encodings: ASCII, UTF-8, UCS-4");
#endif
	AG_SeparatorNewHoriz(win);

	/* Bind to a multilingual AG_Text(3) element. */
	AG_LabelNewS(win, 0, "Multilingual AG_Text(3) buffer:");
	tb = AG_TextboxNewS(win,
	    AG_TEXTBOX_MULTILINGUAL|AG_TEXTBOX_EXPAND|
	    AG_TEXTBOX_MULTILINE|AG_TEXTBOX_EXCL,
	    NULL);
	AG_TextboxBindText(tb, ti->myTxt);
	AG_TextboxSetLang(tb, AG_LANG_FR);
	AG_TextboxSizeHint(tb, "XXXXXXXXXXXXXXXXXXXXXXXXX");
	AG_TextboxSizeHintLines(tb, 5);
	AG_TextboxSetCursorPos(tb, -1);		/* End of string */

	return (0);
}

const AG_TestCase charsetsTest = {
	"charsets",
	N_("Test AG_Editable(3) bound to buffers in different character sets"),
	"1.5.0",
	0,
	sizeof(MyTestInstance),
	Init,
	Destroy,
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

#endif /* AG_UNICODE */
