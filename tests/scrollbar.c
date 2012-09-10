/*	Public domain	*/
/*
 * Test the various bindings provided by the AG_Scrollbar(3) widget using
 * arbitrary values and ranges.
 */

#include "agartest.h"

int vInt = 500;
unsigned vUint = 0;
Sint8 v8 = 50;
Uint32 v32 = 32;
#ifdef AG_HAVE_64BIT
Uint64 v64 = 64;
#endif
float vFlt = 1.0;
double vDbl = 0.0;
#ifdef AG_HAVE_LONG_DOUBLE
long double vLDbl = 1.0;
#endif

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Scrollbar *sb;
	AG_Label *lbl;
	Uint flags = AG_SCROLLBAR_HFILL|AG_SCROLLBAR_NOAUTOHIDE;

	lbl = AG_LabelNewPolled(win, AG_LABEL_HFILL, "Int binding: %d", &vInt);
	AG_LabelSizeHint(lbl, 0, "XXXXXXXXXXXXXXXXXXXX");
	sb = AG_ScrollbarNew(win, AG_SCROLLBAR_HORIZ, flags);
	AG_BindInt(sb, "value", &vInt);
	AG_SetInt(sb, "min", -10000);
	AG_SetInt(sb, "max", 10000);
	AG_SetInt(sb, "inc", 1000);

	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Uint binding: %d (vis=1000)", &vUint);
	sb = AG_ScrollbarNew(win, AG_SCROLLBAR_HORIZ, flags);
	AG_BindUint(sb, "value", &vUint);
	AG_SetUint(sb, "min", 0);
	AG_SetUint(sb, "max", 10000);
	AG_SetUint(sb, "inc", 1000);
	AG_SetUint(sb, "visible", 1000);

	AG_LabelNewPolled(win, AG_LABEL_HFILL, "8-bit binding: %[s8] (vis=50)", &v8);
	sb = AG_ScrollbarNew(win, AG_SCROLLBAR_HORIZ, flags);
	AG_BindSint8(sb, "value", &v8);
	AG_SetSint8(sb, "visible", 50);

	AG_LabelNewPolled(win, AG_LABEL_HFILL, "32-bit binding: %[u32] (vis=10)", &v32);
	sb = AG_ScrollbarNew(win, AG_SCROLLBAR_HORIZ, flags);
	AG_BindUint32(sb, "value", &v32);
	AG_SetUint32(sb, "min", 0);
	AG_SetUint32(sb, "max", 100);
	AG_SetUint32(sb, "visible", 10);

#ifdef AG_HAVE_64BIT
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "64-bit binding: %[u64] (vis=10)", &v64);
	sb = AG_ScrollbarNew(win, AG_SCROLLBAR_HORIZ, flags);
	AG_BindUint64(sb, "value", &v64);
	AG_SetUint64(sb, "min", 0ULL);
	AG_SetUint64(sb, "max", 100ULL);
	AG_SetUint64(sb, "visible", 10ULL);
#endif
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Float binding: %f", &vFlt);
	sb = AG_ScrollbarNew(win, AG_SCROLLBAR_HORIZ, flags);
	AG_BindFloat(sb, "value", &vFlt);
	AG_SetFloat(sb, "min", 0.0f);
	AG_SetFloat(sb, "max", 1.0f);
	AG_SetFloat(sb, "inc", 0.01f);
	
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Double binding: %lf", &vDbl);
	sb = AG_ScrollbarNew(win, AG_SCROLLBAR_HORIZ, flags);
	AG_BindDouble(sb, "value", &vDbl);
	AG_SetDouble(sb, "min", -100.0);
	AG_SetDouble(sb, "max", +100.0);
	AG_SetDouble(sb, "inc", 1.0);

#ifdef AG_HAVE_LONG_DOUBLE
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Long Double binding: %llf", &vLDbl);
	sb = AG_ScrollbarNew(win, AG_SCROLLBAR_HORIZ, flags);
	AG_BindLongDouble(sb, "value", &vLDbl);
#endif
	return (0);
}

const AG_TestCase scrollbarTest = {
	"scrollbar",
	N_("Test the AG_Scrollbar(3) widget"),
	"1.5.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
