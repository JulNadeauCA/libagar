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
SingleLineExample(void)
{
	AG_Window *win;
	AG_Textbox *textbox;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Single-line Example");

	/*
	 * Create a single-line Textbox. TextboxSizeHint() requests an initial
	 * textbox size large enough to display the given string entirely.
	 */
	textbox = AG_TextboxNew(win, 0, "Static string: ");
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXX");
	AG_TextboxPrintf(textbox, "Hello");
	AG_WidgetFocus(textbox);

	/* Bind checkboxes to some flags. */
	AG_SeparatorNewHoriz(win);
	AG_CheckboxNewFn(win, 0, "Disable input", SetDisable, "%p", textbox);
	AG_CheckboxNewFlag(win, 0, "Password input",
	    &textbox->ed->flags, AG_EDITABLE_PASSWORD);
	AG_CheckboxNewFlag(win, 0, "Force integer input",
	    &textbox->ed->flags, AG_EDITABLE_INT_ONLY);
	AG_CheckboxNewFlag(win, 0, "Force float input",
	    &textbox->ed->flags, AG_EDITABLE_FLT_ONLY);
	AG_CheckboxNewFlag(win, 0, "Disable emacs keys",
	    &textbox->ed->flags, AG_EDITABLE_NOEMACS);
	AG_CheckboxNewFlag(win, 0, "Disable traditional LATIN-1",
	    &textbox->ed->flags, AG_EDITABLE_NOLATIN1);

	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_WindowShow(win);
}

static void
MultiLineExample(const char *title, int wordwrap)
{
	AG_Window *win;
	AG_Textbox *textbox;
	char *someText;
	FILE *f;
	size_t size, bufSize;
	unsigned int flags = AG_TEXTBOX_MULTILINE|AG_TEXTBOX_CATCH_TAB;

	if (wordwrap) { flags |= AG_TEXTBOX_WORDWRAP; }

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, title);

	/*
	 * Create a multiline textbox.
	 *
	 * We use the CATCH_TAB flag so that tabs are entered literally in
	 * the string.
	 */
	textbox = AG_TextboxNew(win, flags, NULL);
	AG_Expand(textbox);

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
	AG_TextboxSetCursorPos(textbox, 0);

	AG_CheckboxNewFn(win, 0, "Disable input", SetDisable, "%p", textbox);
#if 0
	AG_SeparatorNewHoriz(win);
	{
		AG_Label *lbl;

		lbl = AG_LabelNewPolled(win, 0, "Lines: %d",
		    &textbox->ed->yMax);
		AG_ExpandHoriz(lbl);

		lbl = AG_LabelNewPolled(win, 0, "Cursor position: %d",
		    &textbox->ed->pos);
		AG_ExpandHoriz(lbl);
	}
#endif
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 540, 380);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("agar-textbox-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	MultiLineExample("Multiline Example (no word wrapping)", 0);
	MultiLineExample("Multiline Example (with word wrapping)", 1);
	SingleLineExample();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}