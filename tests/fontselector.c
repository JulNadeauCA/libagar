/*	Public domain	*/

#include "agartest.h"

static void
SelectedFont(AG_Event *event)
{
/*	AG_Window *win = AG_PTR(1); */

	AG_SetString(agConfig, "font.face", AGOBJECT(agDefaultFont)->name);
	AG_SetInt(agConfig, "font.size", agDefaultFont->spec.size);
	AG_SetUint(agConfig, "font.flags", agDefaultFont->flags);
	if (AG_ConfigSave() == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    "Default font has changed. Restart agartest to apply.");
	} else {
		AG_TextMsgFromError();
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_FontSelector *fs;
	AG_Box *hBox;
	
	fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);
	AG_BindPointer(fs, "font", (void *)&agDefaultFont);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(hBox, 0, _("OK"), SelectedFont, "%p", win);
		AG_ButtonNewFn(hBox, 0, _("Cancel"), AG_WindowCloseGenEv, "%p", win);
	}
	return (0);
}

const AG_TestCase fontSelectorTest = {
	"fontSelector",
	N_("Test the AG_FontSelector(3) widget"),
	"1.5.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
