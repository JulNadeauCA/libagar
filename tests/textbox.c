/*	Public domain	*/
/*
 * This application demonstrates different uses for the Textbox widget.
 */

#include "agartest.h"

#include <agar/core/snprintf.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char bufferShd[60];	/* Shared text buffer */
char bufferExcl[60];	/* Exclusive text buffer */


static void
SetDisable(AG_Event *event)
{
	AG_Textbox *textbox = AG_TEXTBOX_PTR(1);
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
	AG_Textbox *textbox = AG_TEXTBOX_PTR(1);
	int flag = AG_INT(2);

	AG_TextboxSetWordWrap(textbox, flag);
}

static void
SetUppercase(AG_Event *event)
{
	AG_Textbox *textbox = AG_TEXTBOX_PTR(1);
	int flag = AG_INT(2);

	if (flag) {
		textbox->ed->flags |= AG_EDITABLE_UPPERCASE;
	} else {
		textbox->ed->flags &= ~(AG_EDITABLE_UPPERCASE);
	}
}

static void
SetLowercase(AG_Event *event)
{
	AG_Textbox *textbox = AG_TEXTBOX_PTR(1);
	int flag = AG_INT(2);

	if (flag) {
		textbox->ed->flags |= AG_EDITABLE_LOWERCASE;
	} else {
		textbox->ed->flags &= ~(AG_EDITABLE_LOWERCASE);
	}
}

static void
SetString(AG_Event *event)
{
	AG_Textbox *textbox = AG_TEXTBOX_PTR(1);

	AG_TextboxPrintf(textbox, "Formatted string");
	AG_TextboxSetString(textbox, "Set string");
}

static void
DupString(AG_Event *event)
{
	AG_Textbox *textbox = AG_TEXTBOX_PTR(1);
	char *s;

	if ((s = AG_TextboxDupString(textbox)) != NULL) {
		AG_TextMsg(AG_MSG_INFO, "Duplicated string:\n\"%s\"\n", s);
		Free(s);
	} else {
		AG_TextMsgS(AG_MSG_INFO, "Failed");
	}
}

static void
CopyString(AG_Event *event)
{
	char tinybuf[10];
	AG_Textbox *textbox = AG_TEXTBOX_PTR(1);

	AG_TextboxCopyString(textbox, tinybuf, sizeof(tinybuf));
	AG_TextMsg(AG_MSG_INFO, "String truncated to fit %d-byte buffer:\n"
	                        "\"%s\"\n", (int)sizeof(tinybuf), tinybuf);
}

#undef  CB_DEBUG_FLAG
#define CB_DEBUG_FLAG(s,flag)					\
	cb = AG_CheckboxNewFlag(box, 0, s, &ed->flags, (flag));	\
	AGWIDGET(cb)->flags &= ~(AG_WIDGET_FOCUSABLE)
	

static void
DebugStuff(AG_NotebookTab *nt, AG_Textbox *textbox)
{
	AG_Box *box, *hBox;
	AG_Editable *ed = textbox->ed;
	AG_Checkbox *cb;

	AG_SeparatorNewHoriz(nt);

#ifdef AG_ENABLE_STRING
	AG_LabelNewPolled(nt, AG_LABEL_HFILL, "Cursor at: %i", &ed->pos);
	AG_LabelNewPolled(nt, AG_LABEL_HFILL, "Selection: %i", &ed->sel);
	AG_LabelNewPolled(nt, AG_LABEL_HFILL, "x: %i", &ed->x);
#endif
	AG_SeparatorNewHoriz(nt);

	box = AG_BoxNewVert(nt, AG_BOX_EXPAND);
	cb = AG_CheckboxNewFn(box, 0, "Disable input", SetDisable, "%p", textbox);
	AGWIDGET(cb)->flags &= ~(AG_WIDGET_FOCUSABLE);

	CB_DEBUG_FLAG("Blink",               AG_EDITABLE_BLINK_ON);
	CB_DEBUG_FLAG("Read-only",	     AG_EDITABLE_READONLY);
	CB_DEBUG_FLAG("Password input",      AG_EDITABLE_PASSWORD);
	CB_DEBUG_FLAG("Display uppercase",   AG_EDITABLE_UPPERCASE);
	CB_DEBUG_FLAG("Display lowercase",   AG_EDITABLE_LOWERCASE);
	CB_DEBUG_FLAG("Force integer input", AG_EDITABLE_INT_ONLY);
	CB_DEBUG_FLAG("Force float input",   AG_EDITABLE_FLT_ONLY);
	CB_DEBUG_FLAG("Keep cursor in view", AG_EDITABLE_KEEPVISCURSOR);
	CB_DEBUG_FLAG("No emacs keys",       AG_EDITABLE_NOEMACS);
	CB_DEBUG_FLAG("No latin1 keys",      AG_EDITABLE_NOLATIN1);

	AG_SeparatorNewHoriz(nt);

	hBox = AG_BoxNewHoriz(nt, AG_BOX_HFILL);
	{
		AG_ButtonNewFn(hBox, 0, "Set string", SetString, "%p", textbox);
		AG_ButtonNewFn(hBox, 0, "Dup string", DupString, "%p", textbox);
		AG_ButtonNewFn(hBox, 0, "Copy string", CopyString, "%p", textbox);
	}
}

static void
SingleLineExample(AG_Event *event)
{
	static void (*tbBindFn)(AG_Textbox *_Nonnull, char *_Nonnull, AG_Size) =
#ifdef AG_UNICODE
	    AG_TextboxBindUTF8
#else
	    AG_TextboxBindASCII	
#endif
	;
	const char *text = "The Quick Brown Fox Jumps Over The Lazy Dog";
	AG_Window *winParent = AG_WINDOW_PTR(1);
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Textbox *textbox;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, "textbox: Single-line Example");
	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, "Shared #1", AG_BOX_VERT);
	{
		AG_LabelNew(nt, 0, "Bound to the %lu-byte buffer at %p:",
		    (Ulong)sizeof(bufferShd), bufferShd);
		AG_Strlcpy(bufferShd, text, sizeof(bufferShd));

		textbox = AG_TextboxNew(nt, AG_TEXTBOX_HFILL, "Shared buffer: ");
		tbBindFn(textbox, bufferShd, sizeof(bufferShd));

		AG_TextboxSetCursorPos(textbox, -1);	/* To end of string */
		AG_WidgetFocus(textbox);
		DebugStuff(nt, textbox);
	}

	nt = AG_NotebookAdd(nb, "Shared #2", AG_BOX_VERT);
	{
		AG_LabelNew(nt, 0, "Bound to the %lu-byte buffer at %p:",
		    (Ulong)sizeof(bufferShd), bufferShd);

		textbox = AG_TextboxNew(nt, AG_TEXTBOX_HFILL, "Shared buffer: ");
		tbBindFn(textbox, bufferShd, sizeof(bufferShd));
		DebugStuff(nt, textbox);
	}


	/*
	 * Create a single-line Textbox widget with an exclusive binding
	 * to a text buffer. The EXCL option advises that the text buffer is
	 * not bound to any other widget (and its contents are not expected
	 * to change under our feet).
	 */
	nt = AG_NotebookAdd(nb, "Private", AG_BOX_VERT);
	{
		AG_LabelNew(nt, 0, "Bound to the %lu-byte buffer at %p:",
		    (Ulong)sizeof(bufferExcl), bufferExcl);

		AG_Strlcpy(bufferExcl, text, sizeof(bufferExcl));

		textbox = AG_TextboxNew(nt,
		    AG_TEXTBOX_HFILL | AG_TEXTBOX_EXCL,
		    "Exclusive buffer: ");
		tbBindFn(textbox, bufferExcl, sizeof(bufferExcl));
		AG_TextboxSetCursorPos(textbox, -1);	/* To end of string */
		AG_WidgetFocus(textbox);
		DebugStuff(nt, textbox);
	}

#ifdef AG_UNICODE
	/* Create a single-line Textbox bound to an AG_Text object. */
	nt = AG_NotebookAdd(nb, "Multilingual", AG_BOX_VERT);
	{
		AG_Text *txt = AG_TextNew(0);	/* See AG_TextElement(3) */
		
		AG_LabelNew(nt, 0, "Bound to the AG_TextElement(3) at %p:", txt);

		AG_TextSetEntS(txt, AG_LANG_EN, "Hello");
		AG_TextSetEntS(txt, AG_LANG_FR, "Bonjour");
		AG_TextSetEntS(txt, AG_LANG_DE, "Guten Tag");

		textbox = AG_TextboxNew(nt,
		    AG_TEXTBOX_HFILL | AG_TEXTBOX_MULTILINGUAL,
		    "AG_Text element: ");

		AG_TextboxBindText(textbox, txt);
		AG_TextboxSetLang(textbox, AG_LANG_FR);
		AG_TextboxSetCursorPos(textbox, -1);	/* To end of string */
		DebugStuff(nt, textbox);
	}
#endif /* AG_UNICODE */

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
MultiLineExample(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_Window *winParent = AG_WINDOW_PTR(1);
	AG_Window *win;
	AG_Textbox *textbox;
	char *someText;
	FILE *f;
	AG_Size size, bufSize;
	unsigned int flags;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, "textbox: Multi-line example");

	/*
	 * Create a multiline textbox.
	 */
	flags = AG_TEXTBOX_MULTILINE | AG_TEXTBOX_CATCH_TAB |
	        AG_TEXTBOX_EXPAND | AG_TEXTBOX_EXCL;

	textbox = AG_TextboxNewS(win, flags, NULL);

	/*
	 * Load the contents of this file into a buffer. Make the buffer a
	 * bit larger so the user can try entering text.
	 */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "loss.txt", path, sizeof(path)) &&
	    (f = fopen(path, "r")) != NULL) {
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		bufSize = size+1024;
		someText = AG_Malloc(bufSize);
		fread(someText, size, 1, f);
		fclose(f);
		someText[size] = '\0';
	} else {
		someText = AG_Strdup("Failed to load loss.txt");
		bufSize = strlen(someText)+1;
	}

#ifdef AG_UNICODE
	/*
	 * Bind the buffer's contents to the Textbox. The size argument to
	 * AG_TextboxBindUTF8() must include space for the terminating NUL.
	 */
	AG_TextboxBindUTF8(textbox, someText, bufSize);
#else
	AG_TextboxBindASCII(textbox, someText, bufSize);
#endif

	AG_CheckboxNewFlag(win, 0, "Read-only",
	    &textbox->ed->flags, AG_EDITABLE_READONLY);
	AG_CheckboxNewFn(win, 0, "Disable input", SetDisable, "%p", textbox);
	AG_CheckboxNewFn(win, 0, "Word wrapping", SetWordWrap, "%p", textbox);
	AG_CheckboxNewFn(win, 0, "Uppercase", SetUppercase, "%p", textbox);
	AG_CheckboxNewFn(win, 0, "Lowercase", SetLowercase, "%p", textbox);
	AG_SeparatorNewHoriz(win);
	{
#ifdef AG_ENABLE_STRING
		AG_LabelNewPolled(win, AG_LABEL_HFILL,
		    "Lines: %d", &textbox->ed->yMax);
		AG_LabelNewPolled(win, AG_LABEL_HFILL,
		    "Cursor position: %d", &textbox->ed->pos);
#endif
	}
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 540, 380);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_ButtonNewFn(win, 0, "Test multi-line textbox", MultiLineExample, "%p", win);
	AG_ButtonNewFn(win, 0, "Test single-line textbox", SingleLineExample, "%p", win);
	return (0);
}

const AG_TestCase textboxTest = {
	"textbox",
	N_("Test AG_Textbox(3) / AG_Editable(3) widgets"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
