/*	Public domain	*/
/*
 * This application demonstrates use of the AG_Pane container widget.
 */

#include "agartest.h"

static AG_Window *
SettingsWindow(void *obj, AG_Pane *paneHoriz, AG_Pane *paneVert)
{
	const char *actions[] = {
		"EXPAND_DIV1",
		"EXPAND_DIV2",
		"DIVIDE_EVEN",
		"DIVIDE_PCT",
		NULL
	};
	AG_Window *win;

	/* Settings window */
	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, "AG_Pane(3) settings");

	AG_LabelNew(win, 0, "(Horiz. Pane) Resize action:");
	AG_RadioNewUint(win, 0, actions, (void *)&paneHoriz->resizeAction);
		
	AG_LabelNew(win, 0, "(Horiz. Pane) Flags:");
	AG_CheckboxNewFlag(win, 0, "DIV1FILL", &paneHoriz->flags, AG_PANE_DIV1FILL);
	AG_CheckboxNewFlag(win, 0, "FRAME", &paneHoriz->flags, AG_PANE_FRAME);
	AG_CheckboxNewFlag(win, 0, "UNMOVABLE", &paneHoriz->flags, AG_PANE_UNMOVABLE);

	AG_SeparatorNewHoriz(win);

	AG_LabelNew(win, 0, "(Vert. Pane) Resize action:");
	AG_RadioNewUint(win, 0, actions, (void *)&paneVert->resizeAction);
		
	AG_LabelNew(win, 0, "(Vert. Pane) Flags:");
	AG_CheckboxNewFlag(win, 0, "DIV1FILL", &paneVert->flags, AG_PANE_DIV1FILL);
	AG_CheckboxNewFlag(win, 0, "FRAME", &paneVert->flags, AG_PANE_FRAME);
	AG_CheckboxNewFlag(win, 0, "UNMOVABLE", &paneVert->flags, AG_PANE_UNMOVABLE);

	return (win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Pane *paneHoriz, *paneVert;
	AG_Window *winSettings;

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

	if ((winSettings = SettingsWindow(obj, paneHoriz, paneVert)) != NULL) {
		AG_WindowAttach(win, winSettings);
		AG_WindowShow(winSettings);
	}
	return (0);
}

const AG_TestCase paneTest = {
	"pane",
	N_("Test the AG_Pane(3) container widget"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
