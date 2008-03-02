/*	Public domain	*/
/*
 * This program computes the ingredient amounts for a standard black powder
 * formulation. It demonstrates unit conversion and use of widget bindings
 * to control specific variables.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

double kno3=15.0, kno3Min=0.0, kno3Max=750.0;	/* Potassium nitrate */
double c=3.0, cMin=0.0, cMax=750.0;		/* Carbon */
double s=2.0, sMin=0.0, sMax=750.0;		/* Sulfur */

double weight = 0.0;				/* Net weight */
double prSolid = 0.0;				/* Solid products */
double prGaseous = 0.0;				/* Gaseous products */
double prWater = 0.0;				/* Water product */

/* Compute products of reaction. */
static void
UpdateProducts(void)
{
	weight = kno3+c+s;
	prSolid = 55.91*weight/100.0;
	prWater = 1.11*weight/100.0;
	prGaseous = 42.98*weight/100.0;
}

/* Compute KNO3 and C from S. */
static void
UpdateS(AG_Event *event)
{
	kno3 = 7.5*s;
	c = 1.5*s;
	UpdateProducts();
}

/* Compute KNO3 and S from C. */
static void
UpdateC(AG_Event *event)
{
	kno3 = 5.0*c;
	s = (1.0/1.5)*c;
	UpdateProducts();
}

/* Compute S and C from KNO3. */
static void
UpdateKNO3(AG_Event *event)
{
	c = 0.2*kno3;
	s = (1.0/7.5)*kno3;
	UpdateProducts();
}

/* Compute KNO3, S and C from net weight. */
static void
UpdateWeight(AG_Event *event)
{
	kno3 = 75.0*weight/100.0;
	c = 15.0*weight/100.0;
	s = 10.0*weight/100.0;
	UpdateProducts();
}

/* Create our GUI. */
static void
CreateUI(void)
{
	AG_Timeout myTimer;
	AG_Window *win;
	AG_Button *btn;
	AG_Label *lbl;
	AG_Numerical *n1, *n2, *n3, *n;
	AG_Slider *s1, *s2, *s3;
	AG_Box *box;
	int i;

	/* Create a window covering the entire display. */
	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_WindowSetCaption(win, "Gunpowder Calculator");
	AG_WindowSetPadding(win, 10, 10, 10, 10);

	/*
	 * Use AG_Numerical(3) to edit a double-precision floating point
	 * variable.
	 */
	n = AG_NumericalNewDbl(win, AG_NUMERICAL_HFILL, "g",
	    "Net weight: ", &weight);
	AG_NumericalSetMinDbl(n, 0.0);
	AG_NumericalSetMaxDbl(n, 1000.0);
	AG_SetEvent(n, "numerical-changed", UpdateWeight, NULL);

	AG_SeparatorNewHoriz(win);

	/*
	 * Use AG_Slider(3) to edit the specific quantities.
	 */
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		lbl = AG_LabelNew(box, 0, "KNO3: ");
		s1 = AG_SliderNewDbl(box, AG_SLIDER_HORIZ, AG_SLIDER_HFILL,
		    &kno3, &kno3Min, &kno3Max);
		AG_SetEvent(s1, "slider-changed", UpdateKNO3, NULL);
	}
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		lbl = AG_LabelNew(box, 0, "Carbon: ");
		s2 = AG_SliderNewDbl(box, AG_SLIDER_HORIZ, AG_SLIDER_HFILL,
		    &c, &cMin, &cMax);
		AG_SetEvent(s2, "slider-changed", UpdateC, NULL);
	}
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		lbl = AG_LabelNew(box, 0, "Sulfur: ");
		s3 = AG_SliderNewDbl(box, AG_SLIDER_HORIZ, AG_SLIDER_HFILL,
		    &s, &sMin, &sMax);
		AG_SetEvent(s3, "slider-changed", UpdateS, NULL);
	}
	
	AG_SeparatorNewHoriz(win);

	AG_LabelNewStatic(win, 0, "Ingredients:");
	AG_LabelNewPolled(win, AG_LABEL_HFILL,
	    "%Fg KNO3, %Fg C, %Fg S",
	    &kno3, &c, &s);
	AG_LabelNewStatic(win, 0, "Combustion products:");
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
	if (AG_InitCore("agar-gunpowder-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(350, 220, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	CreateUI();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

