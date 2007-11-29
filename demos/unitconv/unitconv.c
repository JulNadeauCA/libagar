/*	Public domain	*/
/*
 * This is a simple unit converter based on AG_Units(3).
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

double value = 0.0;

static void
GetCategories(AG_Tlist *tl)
{
	int i;
	int w, wMax = 0;

	AG_TlistBegin(tl);
	for (i = 0; i < agnUnitGroups; i++) {
		if (strcmp(agUnitGroupNames[i], "Identity") == 0) {
			continue;
		}
		AG_TextSize(agUnitGroupNames[i], &w, NULL);
		if (w > wMax) { wMax = w; }
		AG_TlistAddPtr(tl, NULL, agUnitGroupNames[i], agUnitGroups[i]);
	}
	AG_TlistSelectText(tl, "Length");
	AG_TlistEnd(tl);
	AG_TlistSizeHintPixels(tl, wMax, 6);
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

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Unit Converter");
	AG_WindowSetPadding(win, 10, 10, 10, 10);
	agColors[WINDOW_BG_COLOR] = agColors[BG_COLOR];

	uSel = AG_ComboNew(win, AG_COMBO_HFILL, "Category: ");
	GetCategories(uSel->list);

	AG_SeparatorNewHoriz(win);

	n1 = AG_NumericalNew(win, AG_NUMERICAL_HFILL, "in", "Value: ");
	n2 = AG_NumericalNew(win, AG_NUMERICAL_HFILL, "mm", "Value: ");
	AG_WidgetBindDouble(n1, "value", &value);
	AG_WidgetBindDouble(n2, "value", &value);
	AG_NumericalSizeHint(n1, "0000.00");
	AG_NumericalSizeHint(n2, "0000.00");

	AG_SetEvent(uSel, "combo-selected", SelectCategory, "%p,%p", n1, n2);
	AG_WindowShow(win);
	AG_WindowMaximize(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("unitconv", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(400, 300, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	CreateUI();
	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 60, 60, 60);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

