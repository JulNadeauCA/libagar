/*	Public domain	*/

#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	AG_Timer *tRotHue;
} MyTestInstance;

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->tRotHue = NULL;
	return (0);
}

#ifdef AG_TIMERS
static Uint32
RotateHue(AG_Timer *t, AG_Event *event)
{
	AG_HSVPal *pal = AG_PTR(1);
	float s_save = pal->s;
	float v_save = pal->v;

	pal->h += 1.0f;
	if (pal->h > 360.0f) { pal->h = 0.0f; }
	AG_HSVPal_UpdateSV(pal, pal->triangle.x, pal->triangle.y);
	pal->s = s_save;
	pal->v = v_save;
	return (t->ival);
}

static void
SetRotateHue(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_HSVPal *pal = AG_PTR(2);
	int enable = AG_INT(3);

	if (enable) {
		ti->tRotHue = AG_AddTimerAuto(pal, 10, RotateHue, "%p", pal);
	} else {
		if (ti->tRotHue != NULL)
			AG_DelTimer(pal, ti->tRotHue);
	}
}
#endif /* AG_TIMERS */

static int
TestGUI(void *obj, AG_Window *win)
{
#ifdef AG_TIMERS
	MyTestInstance *ti = obj;
#endif
	AG_HSVPal *pal;

	pal = AG_HSVPalNew(win, AG_HSVPAL_EXPAND);
	pal->h = 240.0f;
	pal->s = 0.80f;
	pal->v = 0.90f;

#ifdef AG_TIMERS
	AG_ButtonNewFn(win, AG_BUTTON_STICKY,
	    "Rotate Hue", SetRotateHue, "%p,%p", ti, pal);
#else
	AG_WidgetDisable(AG_ButtonNewS(win, 0, "Rotate Hue"));
#endif

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	return (0);
}

const AG_TestCase paletteTest = {
	"palette",
	N_("Test the color picker widget"),
	"1.4.2",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
