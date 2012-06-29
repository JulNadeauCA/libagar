/*	Public domain	*/
/*
 * This application demonstrates different uses for the Textbox widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/core/snprintf.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void
SetDisable(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);
	int flag = AG_INT(2);

	if (flag) {
		AG_WidgetDisable(textbox);
	} else {
		AG_WidgetEnable(textbox);
	}
}

static void
SetWordWrap(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);
	int flag = AG_INT(2);

	AG_TextboxSetWordWrap(textbox, flag);
}

static void
DupString(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);
	char *s;

	if ((s = AG_TextboxDupString(textbox)) != NULL) {
		AG_TextMsg(AG_MSG_INFO, "Duplicated string:\n\"%s\"\n", s);
		free(s);
	} else {
		AG_TextMsgS(AG_MSG_INFO, "Failed");
	}
}

static void
CopyString(AG_Event *event)
{
	char tinybuf[10];
	AG_Textbox *textbox = AG_PTR(1);

	AG_TextboxCopyString(textbox, tinybuf, sizeof(tinybuf));
	AG_TextMsg(AG_MSG_INFO, "Copied string (to %d-byte buffer):\n\"%s\"\n",
	    (int)sizeof(tinybuf), tinybuf);
}

static void
DebugStuff(AG_Box *win, AG_Textbox *textbox)
{
	AG_Box *hBox;

	AG_LabelNewPolledMT(win, AG_LABEL_HFILL, &AGOBJECT(textbox->ed)->lock, "Cursor at: %i", &textbox->ed->pos);
	AG_LabelNewPolledMT(win, AG_LABEL_HFILL, &AGOBJECT(textbox->ed)->lock, "Selection: %i", &textbox->ed->sel);
	AG_CheckboxNewFn(win, 0, "Disable input", SetDisable, "%p", textbox);
	AG_CheckboxNewFlag(win, 0, "Read-only", &textbox->ed->flags, AG_EDITABLE_READONLY);
	AG_CheckboxNewFlag(win, 0, "Password input", &textbox->ed->flags, AG_EDITABLE_PASSWORD);
	AG_CheckboxNewFlag(win, 0, "Force integer input", &textbox->ed->flags, AG_EDITABLE_INT_ONLY);
	AG_CheckboxNewFlag(win, 0, "Force float input", &textbox->ed->flags, AG_EDITABLE_FLT_ONLY);
	AG_CheckboxNewFlag(win, 0, "Disable emacs keys", &textbox->ed->flags, AG_EDITABLE_NOEMACS);
	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	AG_ButtonNewFn(hBox, 0, "Dup string", DupString, "%p", textbox);
	AG_ButtonNewFn(hBox, 0, "Copy string", CopyString, "%p", textbox);
}

static void
SingleLineExample(void)
{
	char buffer[60];
	AG_Text *txt;
	AG_Window *win;
	AG_Box *hBox, *vBox;
	AG_Textbox *textbox;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Single-line Example");
	hBox = AG_BoxNewHoriz(win, AG_BOX_EXPAND|AG_BOX_HOMOGENOUS);

	/* Create a single-line Textbox bound to a fixed-size buffer. */
	vBox = AG_BoxNewVert(hBox, AG_BOX_VFILL);
	textbox = AG_TextboxNew(vBox, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL,
	    "Fixed C buffer: ");
	AG_TextboxBindUTF8(textbox, buffer, sizeof(buffer));
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXXXXXXXXXXXX");
	AG_TextboxPrintf(textbox, "Foo bar baz bezo foo");
	AG_TextboxSetCursorPos(textbox, -1);	/* To end of string */
	AG_WidgetFocus(textbox);
	AG_SeparatorNewHoriz(vBox);
	DebugStuff(vBox, textbox);

	/* Create a single-line Textbox bound to an AG_Text object. */
	vBox = AG_BoxNewVert(hBox, AG_BOX_VFILL);
	txt = AG_TextNewS(NULL);
	{
		AG_TextSetEntS(txt, AG_LANG_EN, "Hello");
		AG_TextSetEntS(txt, AG_LANG_FR, "Bonjour");
		AG_TextSetEntS(txt, AG_LANG_DE, "Guten Tag");
	}
	textbox = AG_TextboxNew(vBox, AG_TEXTBOX_HFILL|AG_TEXTBOX_MULTILINGUAL,
	    "AG_Text element: ");
	AG_TextboxBindText(textbox, txt);
	AG_TextboxSetLang(textbox, AG_LANG_EN);
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXXXXXXXXXXXX");
	AG_TextboxSetCursorPos(textbox, -1);	/* To end of string */
	AG_SeparatorNewHoriz(vBox);
	DebugStuff(vBox, textbox);

	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowShow(win);
}

static void
MultiLineExample(const char *title)
{
	AG_Window *win;
	AG_Textbox *textbox;
	char *someText;
	FILE *f;
	size_t size, bufSize;
	unsigned int flags;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, title);

	/*
	 * Create a multiline textbox.
	 */
	flags = AG_TEXTBOX_MULTILINE|AG_TEXTBOX_CATCH_TAB|AG_TEXTBOX_EXPAND|
	        AG_TEXTBOX_EXCL;
	textbox = AG_TextboxNew(win, flags, NULL);

	/*
	 * Load the contents of this file into a buffer. Make the buffer a
	 * bit larger so the user can try entering text.
	 */
	if ((f = fopen("textbox.c", "r")) != NULL) {
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		bufSize = size+1024;
		someText = AG_Malloc(bufSize);
		fread(someText, size, 1, f);
		fclose(f);
		someText[size] = '\0';
	} else {
		someText = AG_Strdup("(Unable to open textbox.c)");
	}

	/*
	 * Bind the buffer's contents to the Textbox. The size argument to
	 * AG_TextboxBindUTF8() must include space for the terminating NUL.
	 */
	AG_TextboxBindUTF8(textbox, someText, bufSize);

	AG_CheckboxNewFlag(win, 0, "Read-only",
	    &textbox->ed->flags, AG_EDITABLE_READONLY);
	AG_CheckboxNewFn(win, 0, "Disable input", SetDisable, "%p", textbox);
	AG_CheckboxNewFn(win, 0, "Word wrapping", SetWordWrap, "%p", textbox);
#if 0
	AG_SeparatorNewHoriz(win);
	{
		AG_Label *lbl;

		lbl = AG_LabelNewPolled(win, AG_LABEL_HFILL,
		    "Lines: %d", &textbox->ed->yMax);

		lbl = AG_LabelNewPolled(win, AG_LABEL_HFILL,
		    "Cursor position: %d", &textbox->ed->pos);
	}
#endif
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 540, 380);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	char *driverSpec = NULL, *optArg;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: textbox [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore(NULL, 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	MultiLineExample("Multiline Example");
	SingleLineExample();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
