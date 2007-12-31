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
CreateTextbox(void)
{
	AG_Timeout myTimer;
	AG_Window *win;
	AG_Textbox *textbox;
	const char *myText;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Textbox Example");

	/*
	 * Create a single-line Textbox and handle the return key with the
	 * ReturnPressed() function. TextboxSizeHint() requests an initial
	 * textbox size large enough to display given string entirely.
	 */
	textbox = AG_TextboxNew(win, AG_TEXTBOX_HFILL, "Static string: ");
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXX");
	AG_SetEvent(textbox, "textbox-return", ReturnPressed, NULL);
	AG_WidgetFocus(textbox);

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

	/*
	 * Create a multiline textbox and configure it to process the
	 * tab key (which normally is used to cycle focus).
	 */
	AG_SeparatorNewHoriz(win);
	AG_LabelNewStatic(win, 0, "Multiline string:");
	textbox = AG_TextboxNew(win,
	    AG_TEXTBOX_MULTILINE|AG_TEXTBOX_EXPAND|AG_TEXTBOX_CATCH_TAB,
	    NULL);
	myText = "struct {\n"
	         "\tint foo;\n"
	         "\tint bar;\n"
	         "}\n";
	AG_TextboxSizeHint(textbox, myText);
	AG_TextboxPrintf(textbox, "%s", myText);

	AG_WindowShow(win);
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
	CreateTextbox();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
