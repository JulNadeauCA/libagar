/*	Public domain	*/
/*
 * This is a simple unit converter based on AG_Units(3).
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

double value = 0.0;

static void
ValueEntered(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();
	
	AG_WidgetFocus(num->input);
}

static void
GetCategories(AG_Tlist *tl)
{
	int i;

	AG_TlistBegin(tl);
	for (i = 0; i < agnUnitGroups; i++) {
		AG_TlistAddPtr(tl, NULL, agUnitGroupNames[i], agUnitGroups[i]);
	}
	AG_TlistSelectText(tl, "Length");
	AG_TlistEnd(tl);
}

static void
SelectCategory(AG_Event *event)
{
	AG_Numerical *n1 = AG_PTR(1);
	AG_Numerical *n2 = AG_PTR(2);
	AG_TlistItem *it = AG_PTR(3);
	AG_Unit *group = it->p1, *unit;
		
	for (unit = &group[0]; unit->key != NULL; unit++) {
		if (unit->divider == 1) {
			printf("Selecting unit: %s\n", unit->key);
			AG_NumericalSetUnitSystem(n1, unit->key);
			AG_NumericalSetUnitSystem(n2, unit->key);
			AG_WindowSetGeometry(AG_WidgetParentWindow(n1),
			    32, 32, agView->w-32, agView->h-32);
			return;
		}
	}
}

static void
CreateUI(void)
{
	AG_Timeout myTimer;
	AG_Window *win;
	AG_Button *btn;
	AG_Label *lbl;
	AG_Numerical *n1, *n2;
	AG_Combo *uSel;
	int i;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_WindowSetCaption(win, "Unit Converter");
	AG_WindowSetPadding(win, 10, 10, 10, 10);
	AG_WindowSetGeometry(win, 32, 32, agView->w-128, agView->h-64);
	agColors[WINDOW_BG_COLOR] = agColors[BG_COLOR];

	uSel = AG_ComboNew(win, AG_COMBO_HFILL, "Category: ");
	GetCategories(uSel->list);

	AG_SeparatorNewHoriz(win);

	n1 = AG_NumericalNew(win, AG_NUMERICAL_HFILL, "in", "Value: ");
	n2 = AG_NumericalNew(win, AG_NUMERICAL_HFILL, "mm", "Value: ");
	AG_WidgetBindDouble(n1, "value", &value);
	AG_WidgetBindDouble(n2, "value", &value);
	AG_NumericalPrescale(n1, "0000.00");
	AG_NumericalPrescale(n2, "0000.00");

	AG_SetEvent(n1, "numerical-return", ValueEntered, NULL);
	AG_SetEvent(n2, "numerical-return", ValueEntered, NULL);
	AG_SetEvent(uSel, "combo-selected", SelectCategory, "%p,%p", n1, n2);

	AG_WidgetFocus(n1->input);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("unitconv", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((c = getopt(argc, argv, "?vfFgGr:")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
		case 'f':
			AG_SetBool(agConfig, "view.full-screen", 1);
			break;
		case 'F':
			AG_SetBool(agConfig, "view.full-screen", 0);
			break;
#ifdef HAVE_OPENGL
		case 'g':
			AG_SetBool(agConfig, "view.opengl", 1);
			break;
		case 'G':
			AG_SetBool(agConfig, "view.opengl", 0);
			break;
#endif
		case 'r':
			fps = atoi(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfFgG] [-r fps]\n", agProgName);
			exit(0);
		}
	}
	if (AG_InitVideo(320, 240, 32, 0) == -1 ||
	    AG_InitInput(0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	CreateUI();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}

