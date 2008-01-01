/*	Public domain	*/
/*
 * This application demonstrates different uses for the Textbox widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/core/snprintf.h>

#include <string.h>

static char polledString[128] = { '\0' };

static void
ReturnPressed(AG_Event *event)
{
	AG_Textbox *textbox = AG_SELF();
	char *s;
	
	/*
	 * Get the current string. Alternatively, AG_TextboxCopyString()
	 * can be used to copy to a fixed-size buffer.
	 */
	s = AG_TextboxDupString(textbox);
	AG_TextMsg(AG_MSG_INFO, "Entered string: `%s'", s);
	free(s);
}

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
SingleLineExample(void)
{
	AG_Timeout myTimer;
	AG_Window *win;
	AG_Textbox *textbox;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Single-line Example");

	/*
	 * Create a single-line Textbox and handle the return key with the
	 * ReturnPressed() function. TextboxSizeHint() requests an initial
	 * textbox size large enough to display given string entirely.
	 */
	textbox = AG_TextboxNew(win, AG_TEXTBOX_HFILL, "Static string: ");
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXX");
	AG_TextboxPrintf(textbox, "Hello");
	AG_SetEvent(textbox, "textbox-return", ReturnPressed, NULL);
	AG_WidgetFocus(textbox);

	/* Bind checkboxes to some flags. */
	AG_CheckboxNewFlag(win, &textbox->flags, AG_TEXTBOX_PASSWORD,
	    "Password input");
	AG_CheckboxNewFlag(win, &AGWIDGET(textbox)->flags, AG_WIDGET_DISABLED,
	    "Disabled input");
	AG_CheckboxNewFlag(win, &textbox->flags, AG_TEXTBOX_INT_ONLY,
	    "Force integer input");
	AG_CheckboxNewFlag(win, &textbox->flags, AG_TEXTBOX_FLT_ONLY,
	    "Force float input");

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
	const char *someText;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Multiline Example");

	/*
	 * Create a multiline textbox and configure it to process the
	 * tab key (which normally is used to cycle focus).
	 */
	AG_LabelNewStatic(win, 0, "Multiline string:");
	textbox = AG_TextboxNew(win,
	    AG_TEXTBOX_MULTILINE|AG_TEXTBOX_EXPAND|AG_TEXTBOX_CATCH_TAB,
	    NULL);
	someText = "struct {\n"
	         "\tint foo;\n"
	         "\tint bar;\n"
	         "}\n";
	AG_TextboxPrintf(textbox, "%s", someText);

	AG_WindowShow(win);
	AG_WindowSetGeometry(win,
	    agView->w/2 - 75, agView->h/2 - 75,
	    150, 150);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("textbox-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(420, 340, 32, AG_VIDEO_RESIZABLE) == -1) {
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
