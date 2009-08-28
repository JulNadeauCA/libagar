/*	Public domain	*/
/*
 * This application tests Agar from C++.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <iostream>

using namespace std;

int
main(int argc, char *argv[])
{
	if (AG_InitCore("cplusplus-demo", 0) == -1) {
		cerr << AG_GetError() << endl;
		return (1);
	}
	if (AG_InitVideo(320, 240, 32, AG_VIDEO_RESIZABLE) == -1) {
		cerr << AG_GetError() << endl;
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_TextMsg(AG_MSG_INFO, "Hello, C++!");
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

