/*	$Csoft: gamemenu.c,v 1.6 2005/10/07 07:09:35 vedge Exp $	*/
/*	Public domain	*/

/*
 * This application demonstrates the use of the AG_Textbox widget for
 * editing a string.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

static char text_buf[128];

static void
return_pressed(AG_Event *event)
{
	AG_Textbox *textbox = AG_SELF();
	char my_string[128];

	/*
	 * Copy the current textbox string to a sized buffer. We could also
	 * have used AG_TextboxDupString() to get a malloc'd copy.
	 */
	AG_TextboxCopyString(textbox, my_string, sizeof(my_string));
	AG_TextMsg(AG_MSG_INFO, "Entered string: `%s'", my_string);
}

static Uint32
update_text_buffer(void *obj, Uint32 ival, void *arg)
{
	snprintf(text_buf, sizeof(text_buf), "Ticks: %lu",
	    (unsigned long)SDL_GetTicks());
	return (ival);
}

static void
CreateTextbox(void)
{
	AG_Timeout update_timer;
	AG_Window *win;
	AG_Button *btn;
	AG_Textbox *textbox;
	int i;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Textbox Example");

	/*
	 * Create a textbox with a label displaying "Enter string:". Use the
	 * `textbox-return' event to catch the return key.
	 */
	textbox = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS,
	    "Enter string: ");
	AG_SetEvent(textbox, "textbox-return", return_pressed, NULL);

	/*
	 * Use a `string' binding to edit the contents of a sized buffer. We
	 * have to pass the size of the buffer along with a pointer to it to
	 * the AG_WidgetBind() function.
	 */
	textbox = AG_TextboxNew(win, AG_TEXTBOX_HFILL, "Edit buffer: ");
	AG_WidgetBind(textbox, "string", AG_WIDGET_STRING, text_buf,
	    sizeof(text_buf));

	/* Use a timer to update the text buffer periodically. */
	snprintf(text_buf, sizeof(text_buf), "Text buffer contents");
	AG_SetTimeout(&update_timer, update_text_buffer, NULL, 0);
	AG_AddTimeout(textbox, &update_timer, 250);

	AG_LabelNew(win, AG_LABEL_POLLED, "Buffer contents: `%s'", &text_buf);

	AG_WindowShow(win);
}

static void
UpdateTable(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	static int prev = 0;
	static int dir = +1;
	int i;

	AG_TableBegin(t);
	for (i = 0; i < prev; i++) {
		AG_TableAddRow(t, "%d:%u", i, (unsigned)SDL_GetTicks());
	}
	AG_TableEnd(t);

	if (dir < 0) {
		if (--prev < 0) { prev = 0; dir = +1; }
	} else {
		if (++prev > 100) { prev = 100; dir = -1; }
	}
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("textbox-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vfFgGr:")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
		case 'f':
			AG_SetBool(agConfig, "view.full-screen", 1);
			break;
		case 'F':
			AG_SetBool(agConfig, "view.full-screen", 0);
			break;
#ifdef HAVE_OPENGL
		case 'g':
			AG_SetBool(agConfig, "view.opengl", 1);
			break;
		case 'G':
			AG_SetBool(agConfig, "view.opengl", 0);
			break;
#endif
		case 'r':
			fps = atoi(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfFgG] [-r fps]\n", agProgName);
			exit(0);
		}
	}

	/* Initialize a 640x480x32 display. Respond to keyboard/mouse events. */
	if (AG_InitVideo(640, 480, 32, 0) == -1 ||
	    AG_InitInput(0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitConfigWin(0);
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F1, KMOD_NONE, AG_ShowSettings);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	CreateTextbox();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}

