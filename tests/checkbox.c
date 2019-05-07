/*	Public domain	*/

#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	Uint myFlags;
} MyTestInstance;

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	Uint mask;
	AG_Box *box;
	int i;

	/* AG_PushStyle("font-size", "120%%"); */
	AG_CheckboxNew(win, 0, "First unbounded checkbox");
	AG_CheckboxNew(win, 0, "Second unbounded checkbox");
	/* AG_PopStyle(); */
	
	AG_LabelNewS(win, 0, "Uint-bounded checkboxes:");
	box = AG_BoxNewVert(win, AG_BOX_FRAME | AG_BOX_EXPAND);
	AG_LabelNewPolled(box, AG_LABEL_EXPAND, "Value: 0x%x", &ti->myFlags);
	for (i = 0, mask = 0x0001; i < 16; i++) {
		AG_CheckboxNewFlag(box, 0,
		    AG_Printf("Bit %d (mask 0x%x)", i+1, mask),
		    &ti->myFlags, mask);
		mask <<= 1;
	}
	return (0);
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->myFlags = 0x4000;
	return (0);
}

const AG_TestCase checkboxTest = {
	"checkbox",
	N_("Test AG_Checkbox(3) widget function"),
	"1.6.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
