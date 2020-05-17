/*	Public domain	*/
/*
 * This application demonstrates different uses for the Textbox widget.
 */

#include "agartest.h"

#include <agar/core/snprintf.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char bufferShd[256];	/* Shared text buffer */
char bufferExcl[256];	/* Exclusive text buffer */

static void
SetDisable(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	const int flag = AG_INT(2);

	if (flag) {
		AG_WidgetDisable(tb);
	} else {
		AG_WidgetEnable(tb);
	}
}

static void
SetWordWrap(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	const int flag = AG_INT(2);

	AG_TextboxSetWordWrap(tb, flag);
}

static void
SetUppercase(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	const int flag = AG_INT(2);

	if (flag) {
		tb->ed->flags |= AG_EDITABLE_UPPERCASE;
	} else {
		tb->ed->flags &= ~(AG_EDITABLE_UPPERCASE);
	}
}

static void
SetLowercase(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	const int flag = AG_INT(2);

	if (flag) {
		tb->ed->flags |= AG_EDITABLE_LOWERCASE;
	} else {
		tb->ed->flags &= ~(AG_EDITABLE_LOWERCASE);
	}
}
static void
ClearString(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);

	AG_TextboxClearString(tb);
}

static void
SetString(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);

	AG_TextboxPrintf(tb, "Hello! %u ticks have passed.", AG_GetTicks());
}

static void
DupString(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	char *s;

	if ((s = AG_TextboxDupString(tb)) != NULL) {
		AG_TextMsg(AG_MSG_INFO, "Duplicated string:\n\"%s\"", s);
		Free(s);
	} else {
		AG_TextMsgS(AG_MSG_INFO, "Failed");
	}
}

static void
TruncateString(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	char *full = AG_TextboxDupString(tb);
	char tinybuf[15];

	AG_TextboxCopyString(tb, tinybuf, sizeof(tinybuf));
	AG_TextMsg(AG_MSG_INFO, "Full string:\n\"%s\"\n\n"
			        "Truncated to fit %d-byte buffer:\n"
	                        "\"%s\"", full, (int)sizeof(tinybuf), tinybuf);
	free(full);
}

#undef  CB_DEBUG_FLAG
#define CB_DEBUG_FLAG(s,flag)					\
	cb = AG_CheckboxNewFlag(box, 0, s, &ed->flags, (flag));	\
	AGWIDGET(cb)->flags &= ~(AG_WIDGET_FOCUSABLE)

static void
DebugStuff(AG_NotebookTab *nt, AG_Textbox *tb)
{
	AG_Box *box, *hBox;
	AG_Editable *ed = tb->ed;
	AG_Checkbox *cb;

	AG_SeparatorNewHoriz(nt);
	AG_LabelNewPolled(nt, AG_LABEL_HFILL, "Cursor at: %i", &ed->pos);
	AG_LabelNewPolled(nt, AG_LABEL_HFILL, "Selection: %i-%i",
	    &ed->selStart, &ed->selEnd);
	AG_LabelNewPolled(nt, AG_LABEL_HFILL, "x: %i", &ed->x);
	AG_SeparatorNewHoriz(nt);

	box = AG_BoxNewVert(nt, AG_BOX_EXPAND);
	cb = AG_CheckboxNewFn(box, 0, "Disable input", SetDisable, "%p", tb);
	AGWIDGET(cb)->flags &= ~(AG_WIDGET_FOCUSABLE);

	CB_DEBUG_FLAG("Blink",                      AG_EDITABLE_BLINK_ON);
	CB_DEBUG_FLAG("Read-only",	            AG_EDITABLE_READONLY);
	CB_DEBUG_FLAG("Password input",             AG_EDITABLE_PASSWORD);
	CB_DEBUG_FLAG("Display uppercase",          AG_EDITABLE_UPPERCASE);
	CB_DEBUG_FLAG("Display lowercase",          AG_EDITABLE_LOWERCASE);
	CB_DEBUG_FLAG("Force integer input",        AG_EDITABLE_INT_ONLY);
	CB_DEBUG_FLAG("Force float input",          AG_EDITABLE_FLT_ONLY);
	CB_DEBUG_FLAG("Keep cursor in view",        AG_EDITABLE_KEEPVISCURSOR);
	CB_DEBUG_FLAG("No Kill and Yank functions", AG_EDITABLE_NO_KILL_YANK);
	CB_DEBUG_FLAG("No alt-key Latin1 mappings", AG_EDITABLE_NO_ALT_LATIN1);

	AG_SeparatorNewHoriz(nt);

	hBox = AG_BoxNewHoriz(nt, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(hBox, 0, "Clear", ClearString,"%p",tb);
		AG_ButtonNewFn(hBox, 0, "Set", SetString,"%p",tb);
		AG_ButtonNewFn(hBox, 0, "Dup", DupString,"%p",tb);
		AG_ButtonNewFn(hBox, 0, "Trunc", TruncateString,"%p",tb);
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	static void (*tbBindFn)(AG_Textbox *_Nonnull, char *_Nonnull, AG_Size) =
#ifdef AG_UNICODE
	    AG_TextboxBindUTF8
#else
	    AG_TextboxBindASCII
#endif
	;
	const char *text = "The Quick Brown Fox Jumps Over The Lazy Dog";
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Textbox *tb, *tb2;

	AG_WindowSetCaptionS(win, "AG_Textbox(3) Example");

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, "Single-line", AG_BOX_VERT);
	{
		AG_LabelNew(nt, 0, "Bound to the %lu-byte buffer at %p:",
		    (Ulong)sizeof(bufferShd), bufferShd);
		AG_Strlcpy(bufferShd, text, sizeof(bufferShd));

		tb = AG_TextboxNew(nt, AG_TEXTBOX_HFILL, "Buffer: ");
		tbBindFn(tb, bufferShd, sizeof(bufferShd));
		AG_SetStyle(tb, "font-size", "120%");
		AG_SetStyle(tb, "padding", "5");

		AG_SpacerNewHoriz(nt);

		tb2 = AG_TextboxNew(nt, AG_TEXTBOX_HFILL, "Buffer (again): ");
		tbBindFn(tb2, bufferShd, sizeof(bufferShd));
		AG_SetStyle(tb2, "font-family", "cm-typewriter");
		AG_SetStyle(tb2, "font-size", "120%");
		AG_SetStyle(tb2, "text-color", "AntiqueWhite");
		AG_SetStyle(tb2, "padding", "0");

//		AG_TextboxSetCursorPos(tb, -1);		/* To end of string */
		AG_WidgetFocus(tb);
		DebugStuff(nt, tb);
	}
	
	nt = AG_NotebookAdd(nb, "Multi-line", AG_BOX_VERT);
	AG_NotebookSelect(nb, nt);
	{
		char path[AG_PATHNAME_MAX];
		char *someText;
		FILE *f;
		AG_Size size, bufSize;

		tb = AG_TextboxNewS(nt, AG_TEXTBOX_MULTILINE |
		                        AG_TEXTBOX_CATCH_TAB |
		                        AG_TEXTBOX_EXPAND |
		                        AG_TEXTBOX_EXCL, NULL);
		AG_TextboxSizeHintPixels(tb, 600, 600);

		/*
		 * Load the contents of this file. Make the buffer a bit
		 * larger so users can try entering text.
		 */
		if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "loss.txt", path, sizeof(path)) &&
		    (f = fopen(path, "r")) != NULL) {
			fseek(f, 0, SEEK_END);
			size = ftell(f);
			fseek(f, 0, SEEK_SET);
			bufSize = size + 4096;
			someText = AG_Malloc(bufSize);
			fread(someText, size, 1, f);
			fclose(f);
			someText[size] = '\0';
		} else {
			someText = AG_Strdup("Failed to load loss.txt");
			bufSize = (AG_Size)strlen(someText)+1;
		}

		/* Connect the Textbox with the buffer. */
#ifdef AG_UNICODE
		AG_TextboxBindUTF8(tb, someText, bufSize);
#else
		AG_TextboxBindASCII(tb, someText, bufSize);
#endif

		AG_CheckboxNewFlag(nt, 0, "ReadOnly", &tb->ed->flags,
		                                       AG_EDITABLE_READONLY);
		AG_CheckboxNewFn(nt, 0, "Disable", SetDisable,"%p",tb);
		AG_CheckboxNewFn(nt, 0, "WordWrap", SetWordWrap,"%p",tb);
		AG_CheckboxNewFn(nt, 0, "Uppercase", SetUppercase,"%p",tb);
		AG_CheckboxNewFn(nt, 0, "Lowercase", SetLowercase,"%p",tb);

		AG_SeparatorNewHoriz(nt);

		AG_LabelNewPolled(nt, 0, "Lines: %d", &tb->ed->yMax);
		AG_LabelNewPolled(nt, 0, "Cursor @ %d", &tb->ed->pos);
	}

	/*
	 * Create a single-line Textbox widget with an exclusive binding
	 * to a text buffer. EXCL advises that the contents of the buffer
	 * are used exclusively by this particular widget instance and are
	 * not expected to change under its feet (thus allowing certain
	 * optimizations to be made).
	 */
	nt = AG_NotebookAdd(nb, "Exclusive", AG_BOX_VERT);
	{
		AG_LabelNew(nt, 0, "Bound to the %lu-byte buffer at %p:",
		    (Ulong)sizeof(bufferExcl), bufferExcl);

		AG_Strlcpy(bufferExcl, text, sizeof(bufferExcl));

		tb = AG_TextboxNew(nt,
		    AG_TEXTBOX_HFILL | AG_TEXTBOX_EXCL,
		    "Exclusive buffer: ");
		tbBindFn(tb, bufferExcl, sizeof(bufferExcl));
		AG_TextboxSetCursorPos(tb, -1);	/* To end of string */
		DebugStuff(nt, tb);
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

		tb = AG_TextboxNew(nt,
		    AG_TEXTBOX_HFILL | AG_TEXTBOX_MULTILINGUAL,
		    "AG_Text element: ");

		AG_TextboxBindText(tb, txt);
		AG_TextboxSetLang(tb, AG_LANG_FR);
		AG_TextboxSetCursorPos(tb, -1);	/* To end of string */
		DebugStuff(nt, tb);
	}
#endif /* AG_UNICODE */

	return (0);
}

const AG_TestCase textboxTest = {
	"textbox",
	N_("Test the AG_Textbox(3) / AG_Editable(3) widget"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
