/*	Public domain	*/
/*
 * This program computes the ingredient amounts for a standard black
 * powder formulation.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

double kno3 = 15.0;
double c = 3.0;
double s = 2.0;
double weight = 0.0, prSolid = 0.0, prWater = 0.0, prGaseous = 0.0;

static void
UpdateProducts(void)
{
	weight = kno3+c+s;
	prSolid = 55.91*weight/100.0;
	prWater = 1.11*weight/100.0;
	prGaseous = 42.98*weight/100.0;
}

static void
UpdateS(AG_Event *event)
{
	kno3 = 7.5*s;
	c = 1.5*s;
	UpdateProducts();
}

static void
UpdateC(AG_Event *event)
{
	kno3 = 5.0*c;
	s = (1.0/1.5)*c;
	UpdateProducts();
}

static void
UpdateWeight(AG_Event *event)
{
	kno3 = 75.0*weight/100.0;
	c = 15.0*weight/100.0;
	s = 10.0*weight/100.0;
	UpdateProducts();
}

static void
UpdateKNO3(AG_Event *event)
{
	c = 0.2*kno3;
	s = (1.0/7.5)*kno3;
	UpdateProducts();
}

static void
CreateUI(void)
{
	AG_Timeout myTimer;
	AG_Window *win;
	AG_Button *btn;
	AG_Label *lbl;
	AG_Numerical *n1, *n2, *n3, *n;
	int i;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_WindowSetCaption(win, "Gunpowder Calculator");
	AG_WindowSetPadding(win, 10, 10, 10, 10);
	
	n = AG_NumericalNew(win, AG_NUMERICAL_HFILL, "g", "Total weight: ");
	AG_WidgetBindDouble(n, "value", &weight);
	AG_SetEvent(n, "numerical-changed", UpdateWeight, NULL);

	AG_SeparatorNewHoriz(win);

	n1 = AG_NumericalNew(win, AG_NUMERICAL_HFILL, "g", "KNO3: ");
	n2 = AG_NumericalNew(win, AG_NUMERICAL_HFILL, "g", "Carbon: ");
	n3 = AG_NumericalNew(win, AG_NUMERICAL_HFILL, "g", "Sulfur: ");
	AG_WidgetBindDouble(n1, "value", &kno3);
	AG_WidgetBindDouble(n2, "value", &c);
	AG_WidgetBindDouble(n3, "value", &s);
	AG_NumericalSizeHint(n1, "0000.00");
	AG_NumericalSizeHint(n2, "0000.00");
	AG_NumericalSizeHint(n3, "0000.00");
	AG_SetEvent(n1, "numerical-changed", UpdateKNO3, NULL);
	AG_SetEvent(n2, "numerical-changed", UpdateC, NULL);
	AG_SetEvent(n3, "numerical-changed", UpdateS, NULL);
	
	AG_SeparatorNewHoriz(win);

	AG_LabelNewStatic(win, 0, "Products of reaction:");
	AG_LabelNewPolled(win, AG_LABEL_HFILL,
	    "%Fg solid, %Fg gaseous, %Fg H2O",
	    &prSolid, &prGaseous, &prWater);
	
	UpdateProducts();

	AG_WindowShow(win);
	AG_WindowMaximize(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("Gunpowder Calculator", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(250, 170, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_SetRefreshRate(-1);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	CreateUI();
	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}

