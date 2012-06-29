/*	Public domain	*/
/*
 * Test the ability of AG_Editable(3) to bind to string buffers in
 * different character set encodings.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

#include <agar/config/have_iconv.h>

int
main(int argc, char *argv[])
{
	char myASCII[20*4];
	char myUTF[30*4];
	char myLat1[30*4];
	AG_Text *myTxt;
	char *driverSpec = NULL, *optArg;
	AG_Window *win;
	AG_Textbox *tb;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: charsets [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore(NULL, AG_CORE_VERBOSE) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Character set conversion test");
	
	/* Bind to a C string in US-ASCII */
	AG_Strlcpy(myASCII, "ASCII!", sizeof(myASCII));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL, "ASCII Buffer: ");
	AG_TextboxBindASCII(tb, myASCII, sizeof(myASCII));
	
	/* Bind to a C string in UTF-8 */
	AG_Strlcpy(myUTF, "\xc3\x85ngstrom!", sizeof(myUTF));
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL, "UTF-8 Buffer: ");
	AG_TextboxBindUTF8(tb, myUTF, sizeof(myUTF));

#ifdef HAVE_ICONV
	AG_Strlcpy(myLat1, "Overv\xE5knign for feils\xF8king!", sizeof(myLat1));
	/* Bind to a C string in LATIN-1 */
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
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL,
	    "AG_Text Buffer: ");
	AG_TextboxBindText(tb, myTxt);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Quit", AGWINDETACH(win));
	
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, -1);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

