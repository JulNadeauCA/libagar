/*	Public domain	*/

#include "agartest.h"

static void
LookupUser(AG_Event *event)
{
	char name[AG_USER_NAME_MAX];
	AG_Textbox *tb = AG_TEXTBOX_SELF();
	AG_User *u;

	AG_TextboxCopyString(tb, name, sizeof(name));

	if ((u = AG_GetUserByName(name)) != NULL) {
		AG_TextMsg(AG_MSG_INFO,
		    "Search \"%s\" returned:\n"
		    "%s%s\n"
		    "uid = %lu, gid=%lu\n"
		    "gecos = %s\n"
		    "home  = %s\n"
		    "shell = %s\n"
		    "tmp   = %s",
		    name,
		    u->name,
		    (u->flags & AG_USER_NO_ACCOUNT) ? " [fake]" : "",
		    (unsigned long)u->uid,
		    (unsigned long)u->gid,
		    u->gecos, u->home, u->shell, u->tmp);
	} else {
		AG_TextMsgFromError();
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_User *uEff;
	AG_Textbox *tb;
	AG_Label *lbl;

	
	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, _("AG_User(3) test"));
	AG_SetStyle(lbl, "font-size", "120%");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);
	lbl = AG_LabelNew(win, AG_LABEL_HFILL, _("Backend: %s"), agUserOps->name);
	AG_LabelJustify(lbl, AG_TEXT_CENTER);
	
	AG_SeparatorNewHoriz(win);
	
	if ((uEff = AG_GetEffectiveUser()) != NULL) {
		AG_LabelNew(win, 0, _("Effective user: %s (%lu:%lu)"),
		    uEff->name,
		    (unsigned long)uEff->uid,
		    (unsigned long)uEff->gid);
	} else {
		AG_LabelNewS(win, 0, AG_GetError());
	}

	tb = AG_TextboxNewS(win, AG_TEXTBOX_HFILL, _("Lookup user: "));
	AG_SetEvent(tb, "textbox-return", LookupUser, NULL);
	AG_WidgetFocus(tb);

	return (0);
}

const AG_TestCase userTest = {
	"user",
	N_("Test the AG_User(3) interface"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
