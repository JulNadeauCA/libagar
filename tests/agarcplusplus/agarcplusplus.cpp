/*	Public domain	*/
/*
 * This program tests that the various Agar headers are compiling
 * cleanly under C++.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

#include <agar/rg.h>
#include <agar/vg.h>

#include <iostream>

using namespace std;

int
main(int argc, char *argv[])
{
	AG_Window *win;

	if (AG_InitCore("cplusplus-demo", 0) == -1) {
		cerr << AG_GetError() << endl;
		return (1);
	}
	if (AG_InitGraphics(NULL) == -1) {
		cerr << AG_GetError() << endl;
		return (-1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);

	win = AG_WindowNew(0);
	AG_LabelNew(win, 0, "Hello, C++!");
	AG_WindowShow(win);
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

