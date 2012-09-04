/*	Public domain	*/
/*
 * Test the various bindings provided by the AG_Scrollbar(3) widget using
 * arbitrary values and ranges.
 */

#include "agartest.h"

int vInt = 50000, minInt = -100000, maxInt = 100000, visInt = 0;
unsigned vUint = 0, minUint = 0, maxUint = 10, visUint = 0;
Sint8 v8 = 100, min8 = -100, max8 = 100, vis8 = 0;
Uint32 v32 = 1234, min32 = 1, max32 = 1000000, vis32 = 0;
#ifdef AG_HAVE_64BIT
Uint64 v64 = 1234, min64 = 1, max64 = 1000000, vis64 = 0;
#endif
float vFlt = 1.0, minFlt = 0.0, maxFlt = 10.0, visFlt = 0.0;
double vDbl = 1.0, minDbl = -1e6, maxDbl = 1e6, visDbl = 0.0;
#ifdef AG_HAVE_LONG_DOUBLE
long double vLDbl = 1.0, minLDbl = -1e6, maxLDbl = 1e6, visLDbl = 0.0;
#endif

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Scrollbar *sb;

	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Int binding: %d", &vInt);
	sb = AG_ScrollbarNewInt(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &vInt, &minInt, &maxInt, &visInt);
	AG_ScrollbarSetIntIncrement(sb, 100);

	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Uint binding: %d", &vUint);
	sb = AG_ScrollbarNewUint(win, AG_SCROLLBAR_HORIZ, AG_LABEL_HFILL,
	    &vUint, &minUint, &maxUint, &visUint);

	AG_LabelNewPolled(win, AG_LABEL_HFILL, "8-bit binding: %[s8]", &v8);
	sb = AG_ScrollbarNewSint8(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &v8, &min8, &max8, &vis8);
	
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "32-bit binding: %[u32]", &v32);
	sb = AG_ScrollbarNewUint32(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &v32, &min32, &max32, &vis32);
#ifdef AG_HAVE_64BIT
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "64-bit binding: %[u64]", &v64);
	sb = AG_ScrollbarNewUint64(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &v64, &min64, &max64, &vis64);
#endif
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Float binding: %f", &vFlt);
	sb = AG_ScrollbarNewFloat(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &vFlt, &minFlt, &maxFlt, &visFlt);
	
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Double binding: %lf", &vDbl);
	sb = AG_ScrollbarNewDouble(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &vDbl, &minDbl, &maxDbl, &visDbl);
#ifdef AG_HAVE_LONG_DOUBLE
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Long Double binding: %llf", &vLDbl);
	sb = AG_ScrollbarNewLongDouble(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &vLDbl, &minLDbl, &maxLDbl, &visLDbl);
#endif
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, -1);
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
