/*	Public domain	*/
/*
 * This application tests Agar from C++.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <iostream>

using namespace std;

int
main()
{
	if (AG_InitCore("cplusplus-demo", 0) == -1) {
		cerr << AG_GetError() << endl;
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		cerr << AG_GetError() << endl;
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	AG_TextMsg(AG_MSG_INFO, "Hello, world!");

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

