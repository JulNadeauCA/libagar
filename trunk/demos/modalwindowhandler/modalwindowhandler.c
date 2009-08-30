/*	Public domain	*/

#include <agar/core.h>
#include <agar/gui.h>

void
PopupReact(AG_Event *event)
{
	AG_TextPromptString("This is the second popup prompt.",
	    PopupReact, NULL);
}

int
main(int argc, char *argv[])
{
	AG_Window *win;

	if (AG_InitCore("agar-windowhandler-demo", 0) == -1 ||
	    AG_InitVideo(320, 240, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);

	AG_TextPromptString("This is the first popup prompt.",
	    PopupReact, NULL);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}
