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
#include <errno.h>

static char polledString[128] = { '\0' };

/* Callback function for our test timer. */
static Uint32
UpdateText(void *obj, Uint32 ival, void *arg)
{
	AG_Textbox *textbox = arg;

	if (!AG_WidgetFocused(textbox)) {
		AG_Snprintf(polledString, sizeof(polledString), "Tick: %lu",
		    (unsigned long)SDL_GetTicks());
	}
	return (ival);
}

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
SetStaticFlag(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);
	int flag = AG_INT(2);

	AG_TextboxSetStatic(textbox, flag);
}

static void
SingleLineExample(void)
{
	AG_Timeout myTimer;
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

	AG_SeparatorNewHoriz(win);

	/*
	 * Use a `string' binding to edit the contents of a sized buffer. We
	 * have to pass the size of the buffer along with a pointer to it to
	 * the AG_WidgetBind() function.
	 *
	 * If this were a multithreaded application, the buffer could be
	 * protected with a mutex and we would use AG_WidgetBindMp().
	 */
	textbox = AG_TextboxNew(win, AG_TEXTBOX_HFILL, "Polled string: ");
	AG_TextboxSizeHint(textbox, "Tick: 000000");
	AG_WidgetBind(textbox, "string", AG_WIDGET_STRING, polledString,
	    sizeof(polledString));

	/* Use a timer to update the text buffer periodically. */
	AG_SetTimeout(&myTimer, UpdateText, textbox, 0);
	AG_AddTimeout(NULL, &myTimer, 250);

	/* Create a polled label to display the actual buffer contents. */
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Polled string: <%s>",
	    &polledString);

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
	size_t size;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Multiline Example");

	/*
	 * Create a multiline textbox.
	 *
	 * We use the CATCH_TAB flag so that tabs are entered literally in
	 * the string. The STATIC flag enables important optimizations based
	 * on the assumption that the string will be edited by this Textbox
	 * only and will not change under its feet.
	 */
	textbox = AG_TextboxNew(win,
	    AG_TEXTBOX_MULTILINE|AG_TEXTBOX_EXPAND|AG_TEXTBOX_CATCH_TAB|
	    AG_TEXTBOX_STATIC,
	    NULL);

	/* Load the contents of this file into a buffer. */
	if ((f = fopen("textbox.c", "r")) != NULL) {
		fseek(f, 0, SEEK_END); size = ftell(f); fseek(f, 0, SEEK_SET);
		someText = AG_Malloc(size+1);
		fread(someText, size, 1, f);
		fclose(f);
		someText[size] = '\0';
	} else {
		someText = AG_Strdup(strerror(errno));
	}

	/*
	 * Bind the buffer's contents to the Textbox. The size argument to
	 * WidgetBindString() must include space for the terminating NUL.
	 */
	AG_WidgetBindString(textbox, "string", someText, size+1);
	AG_TextboxSetCursorPos(textbox, 0);

	AG_CheckboxNewFn(win, 0, "Disable input",
	    DisableInput, "%p", textbox);
	AG_CheckboxNewFn(win, AG_CHECKBOX_SET, "Static optimizations",
	    SetStaticFlag, "%p", textbox);
	AG_SeparatorNewHoriz(win);
	AG_LabelNewPolled(win, AG_LABEL_HFILL,
	    "Lines: %d", &textbox->ed->yMax);
	AG_LabelNewPolled(win, AG_LABEL_HFILL,
	    "Cursor position: %d", &textbox->ed->pos);

	AG_WindowShow(win);
	AG_WindowSetGeometry(win,
	    agView->w/2 - 540/2, agView->h/2 - 380/2,
	    540, 380);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("textbox-demo", 0) == -1) {
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
