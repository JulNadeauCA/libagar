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
DisableInput(AG_Event *event)
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
	textbox = AG_TextboxNew(win, AG_TEXTBOX_HFILL, "Static string: ");
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXX");
	AG_TextboxPrintf(textbox, "Hello");
	AG_WidgetFocus(textbox);

	/* Bind checkboxes to some flags. */
	AG_SeparatorNewHoriz(win);
	AG_CheckboxNewFn(win, 0, "Disable input", DisableInput, "%p", textbox);
	AG_CheckboxNewFlag(win, &textbox->ed->flags, AG_EDITABLE_PASSWORD,
	    "Password input");
	AG_CheckboxNewFlag(win, &textbox->ed->flags, AG_EDITABLE_INT_ONLY,
	    "Force integer input");
	AG_CheckboxNewFlag(win, &textbox->ed->flags, AG_EDITABLE_FLT_ONLY,
	    "Force float input");
	AG_CheckboxNewFlag(win, &textbox->ed->flags, AG_EDITABLE_NOEMACS,
	    "Disable emacs keys");
	AG_CheckboxNewFlag(win, &textbox->ed->flags, AG_EDITABLE_NOLATIN1,
	    "Disable traditional LATIN-1");

	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_WindowShow(win);
}

static void
MultiLineExample(void)
{
	AG_Window *win;
	AG_Textbox *textbox;
	char *someText;
	FILE *f;
	size_t size, bufSize;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Multiline Example");

	/*
	 * Create a multiline textbox.
	 *
	 * We use the CATCH_TAB flag so that tabs are entered literally in
	 * the string.
	 */
	textbox = AG_TextboxNew(win,
	    AG_TEXTBOX_MULTILINE|AG_TEXTBOX_EXPAND|AG_TEXTBOX_CATCH_TAB,
	    NULL);

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

	AG_CheckboxNewFn(win, 0, "Disable input",
	    DisableInput, "%p", textbox);
	AG_SeparatorNewHoriz(win);
	AG_LabelNewPolled(win, AG_LABEL_HFILL,
	    "Lines: %d", &textbox->ed->yMax);
	AG_LabelNewPolled(win, AG_LABEL_HFILL,
	    "Cursor position: %d", &textbox->ed->pos);

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
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	MultiLineExample();
	SingleLineExample();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
