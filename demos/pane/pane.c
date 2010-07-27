/*	Public domain	*/
/*
 * This application demonstrates use of the AG_Pane container widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;
	AG_Pane *paneHoriz, *paneVert;

	if (AG_InitCore("agar-pane-demo", 0) == -1 ||
	    AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar Pane demo");

	/* Divide the window horizontally */
	paneHoriz = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	{
		/* Divide the left pane vertically */
		paneVert = AG_PaneNewVert(paneHoriz->div[0], AG_PANE_EXPAND);
		AG_LabelNew(paneVert->div[0], 0, "Left/Top");
		AG_LabelNew(paneVert->div[1], 0, "Left/Bottom");
		AG_PaneMoveDividerPct(paneVert, 30);
	}
	AG_LabelNew(paneHoriz->div[1], 0, "Right");
	AG_PaneMoveDividerPct(paneHoriz, 50);
	AG_WindowSetGeometry(win, -1, -1, 320, 240);
	AG_WindowShow(win);

	/* Settings window */
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Pane settings");
	{
		const char *actions[] = {
			"EXPAND_DIV1",
			"EXPAND_DIV2",
			"DIVIDE_EVEN",
			"DIVIDE_PCT",
			NULL
		};
		AG_Radio *rad;

		AG_LabelNew(win, 0, "(Horiz. Pane) Resize action:");
		rad = AG_RadioNewUint(win, 0, actions,
		    &paneHoriz->resizeAction);
		
		AG_LabelNew(win, 0, "(Horiz. Pane) Flags:");
		AG_CheckboxNewFlag(win, 0, "DIV1FILL", &paneHoriz->flags,
		    AG_PANE_DIV1FILL);
		AG_CheckboxNewFlag(win, 0, "FRAME", &paneHoriz->flags,
		    AG_PANE_FRAME);
		AG_CheckboxNewFlag(win, 0, "UNMOVABLE", &paneHoriz->flags,
		    AG_PANE_UNMOVABLE);
	
		AG_SeparatorNewHoriz(win);

		AG_LabelNew(win, 0, "(Vert. Pane) Resize action:");
		rad = AG_RadioNewUint(win, 0, actions,
		    &paneVert->resizeAction);
		
		AG_LabelNew(win, 0, "(Vert. Pane) Flags:");
		AG_CheckboxNewFlag(win, 0, "DIV1FILL", &paneVert->flags,
		    AG_PANE_DIV1FILL);
		AG_CheckboxNewFlag(win, 0, "FRAME", &paneVert->flags,
		    AG_PANE_FRAME);
		AG_CheckboxNewFlag(win, 0, "UNMOVABLE", &paneVert->flags,
		    AG_PANE_UNMOVABLE);

		AG_WindowShow(win);
	}

	AG_EventLoop();
	AG_Destroy();
	return (0);
}
