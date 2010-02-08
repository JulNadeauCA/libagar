/*	Public domain	*/

#include <agar/core.h>
#include <agar/gui.h>
#include <stdio.h>

static int
DemoFail(const char *s)
{
	if (AG_InitCore("agardemostub", AG_NO_CFG_AUTOLOAD) == -1 ||
	    AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "Agar initialization failed: %s\n",
		    AG_GetError());
		return (1);
	}
	fprintf(stderr, "%s", s);
	AG_TextMsgS(AG_MSG_INFO, s);
	AG_EventLoop();
	AG_Destroy();
	return (0);
}
