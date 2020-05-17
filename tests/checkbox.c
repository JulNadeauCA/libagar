/*	Public domain	*/

#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	Uint myFlags;
} MyTestInstance;

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Box *box;
	AG_Label *lbl;
	MyTestInstance *ti = obj;
	Uint mask;
	int i;

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, "Checkbox test");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);
	AG_SetStyle(lbl, "font-size", "200%");

	AG_CheckboxNew(win, 0, "First unbounded checkbox");
	AG_CheckboxNew(win, 0, "Second unbounded checkbox");
	AG_SeparatorNewHoriz(win);
	AG_NumericalNewUint(win, 0, NULL, "Bounded value: ", &ti->myFlags);
	AG_SpacerNewHoriz(win);

	AG_LabelNewS(win, 0, "Bounded checkboxes (bits):");
	box = AG_BoxNewVert(win, AG_BOX_EXPAND);
	AG_LabelNewPolled(box, AG_LABEL_EXPAND, "Value: 0x%x", &ti->myFlags);
	for (i = 0, mask = 0x0001; i < 16; i++) {
		AG_CheckboxNewFlag(box, 0,
		    AG_Printf("Bit %d (mask 0x%x)", i+1, mask),
		    &ti->myFlags, mask);
		mask <<= 1;
	}

	AG_SpacerNewHoriz(win);
	AG_CheckboxNewUint(win, 0, "Bounded checkbox (as bool)", &ti->myFlags);

	return (0);
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->myFlags = 1;
	return (0);
}

const AG_TestCase checkboxTest = {
	"checkbox",
	N_("Test multiple AG_Checkbox(3) bound to a same variable"),
	"1.6.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
