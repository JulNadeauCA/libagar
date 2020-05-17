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
	AG_HSVPal *pal = AG_HSVPAL_PTR(1);
	const float s_save = pal->s;
	const float v_save = pal->v;

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
	AG_HSVPal *pal = AG_HSVPAL_PTR(2);
	const int enable = AG_INT(3);

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
	AG_Box *box;

	pal = AG_HSVPalNew(win, AG_HSVPAL_EXPAND | AG_HSVPAL_SHOW_RGB);
	pal->h = 240.0f;
	pal->s = 0.80f;
	pal->v = 0.90f;

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
#ifdef AG_TIMERS
		AG_ButtonNewFn(box, AG_BUTTON_STICKY, "Rotate Hue",
		    SetRotateHue, "%p,%p", ti, pal);
#endif
		AG_ButtonNewFlag(box, AG_BUTTON_STICKY, "Show RGB",
		    &pal->flags, AG_HSVPAL_SHOW_RGB);
		AG_ButtonNewFlag(box, AG_BUTTON_STICKY, "Show HSV",
		    &pal->flags, AG_HSVPAL_SHOW_HSV);
	}
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFlag(box, AG_BUTTON_STICKY, "No Alpha",
		    &pal->flags, AG_HSVPAL_NOALPHA);
		AG_ButtonNewFlag(box, AG_BUTTON_STICKY, "No Preview",
		    &pal->flags, AG_HSVPAL_NOPREVIEW);
	}

/*	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 500, 500); */
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
