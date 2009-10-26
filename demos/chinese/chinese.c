/*	Public domain	*/
/*
 * This program tests Chinese text rendering.
 */

#include <agar/core.h>
#include <agar/gui.h>

#define TESTFONT "HAN NOM B.ttf"

int
main(int argc, char *argv[])
{
	AG_Font *font;

	/* Initialize Agar-GUI. */
	if (AG_InitCore("agar-chinese-demo", 0) == -1 ||
	    AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	if (!AG_FileExists(TESTFONT)) {
		AG_TextMsg(AG_MSG_ERROR,
		    "This demo requires the font \"%s\","
		    "please download it from:\n"
		    "http://vietunicode.sourceforge.net/fonts/fonts_hannom.html\n"
		    "and place it in the current directory.", TESTFONT);
	} else {
		AG_SetString(agConfig, "font-path", ".");

		if ((font = AG_FetchFont(TESTFONT, 18, 0)) != NULL) {
			AG_SetDefaultFont(font);
			AG_TextMsg(AG_MSG_INFO,
			    "\xE6\xB1\x89\xE8\xAF\xAD\x2F"
			    "\xE6\xBC\xA2\xE8\xAA\x9E\x0A");
		} else {
			AG_TextMsg(AG_MSG_ERROR, "%s: %s",
			    TESTFONT, AG_GetError());
		}
	}

	/* Use the standard Agar event loop. */
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
