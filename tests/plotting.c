/*	Public domain	*/

/*
 * This program shows a practical use for the M_Plotter(3) widget.
 * It computes an optimal "squared-sine" velocity profile, and plots
 * the derivatives.
 */

#include "agartest.h"

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
	AG_TestInstance _inherit;	/* AG_TestInstance -> MyTestInstance */

	M_Real L;			/* Length of travel */
	M_Real F;			/* Target feed rate */
	M_Real Amax;			/* Acceleration limit */
	M_Real Jmax;			/* Jerk limit */

	M_Real uTs, uTa, uTo;
	M_Plot *plVel, *plAcc, *plJerk;
	M_PlotLabel *plblCase;
	
	/* Generated constants for squared sine */
	M_Real Aref;
	M_Real v, k;
	M_Real Ts, Ta, To;
	M_Real t1, t2, t3, t4, t5, t6, t7;
	M_Real v1, v2, v3;
} MyTestInstance;

/*
 * Compute the constants used in the squared sine algorithm, given the
 * parameters L (displacement), F, Amax and Jmax.
 */
static void
ComputeSquaredSineConstants(MyTestInstance *ti)
{
	const char *which = NULL;
	const M_Real L = ti->L;
	const M_Real F = ti->F;
	const M_Real Amax = ti->Amax;
	const M_Real Jmax = ti->Jmax;

	/*
	 * Compute the shortest amount of time needed for the squared
	 * sine velocity profile for the given feedrate, under constraints
	 * of maximum acceleration and maximum jerk.
	 */
	if (F >= M_PI_2*((Amax*Amax)/Jmax) ) {
		/*
		 * Feedrate F is achievable by using maximum acceleration
		 * and maximum jerk.
		 */
		if (L >= ( (F*F)/Amax + M_PI_2*((Amax*F)/Jmax) )){
			/* Feedrate F is achievable at Amax acceleration. */
			which = "1.1";
			ti->Ts = M_PI_2*(Amax/Jmax);
			ti->Ta = (F/Amax) + ti->Ts;
			ti->To = L/F;
		} else if (L <  ( (F*F)/Amax + M_PI_2*((Amax*F)/Jmax) ) &&
	      		   L >= ( (M_PI*M_PI)/2.0) * (Amax*Amax*Amax) /
			                             (Jmax*Jmax) ) {
			/* Feedrate F is unachievable, Amax is achievable. */
			which = "1.2";
			ti->Ts = M_PI_2*(Amax/Jmax);
			ti->Ta = (( -M_PI*(Amax*Amax) +
			       sqrt((M_PI*M_PI)*(Amax*Amax*Amax*Amax) +
			            16.0*Amax*(Jmax*Jmax)*L)
			     ) /
			     ( 4.0*Jmax*Amax )) + ti->Ts;
			ti->To = ti->Ta;
		} else if (L <= ( ((M_PI*M_PI)/2.0)*(Amax*Amax*Amax) /
		                                    (Jmax*Jmax) )) {
			/* Neither F nor Amax are achievable. */
			which = "1.3";
			ti->Ts = pow( (M_PI*L)/(4.0*Jmax), 1.0/3.0 );
			ti->Ta = 2.0*ti->Ts;
			ti->To = ti->Ta;
		} else {
			return;
		}
	} else if (F < M_PI_2*((Amax*Amax)/Jmax) ) {
		/*
		 * Feedrate F is achievable without using maximum
		 * acceleration.
		 */
		if (L >= sqrt( (2.0*M_PI*(F*F*F))/Jmax ) ) {
			/* F is achievable, Amax is unachievable. */
			which = "2.1";
			ti->Ts = sqrt( (M_PI*F)/(2.0*Jmax) );
			ti->Ta = 2.0*ti->Ts;
			ti->To = L/F;
		} else if (L < sqrt( (2.0*M_PI*(F*F*F))/Jmax )) {
			/* Neither F nor Amax are achievable. */
			which = "2.2";
			ti->Ts = pow( (M_PI*L)/(4.0*Jmax), 1.0/3.0 );
			ti->Ta = 2.0*ti->Ts;
			ti->To = ti->Ta;
		} else {
			return;
		}
	}
	if (ti->uTs != 0.0) { ti->Ts = ti->uTs; }
	if (ti->uTa != 0.0) { ti->Ta = ti->uTa; }
	if (ti->uTo != 0.0) { ti->To = ti->uTo; }

	ti->k = M_PI/(2.0*ti->Ts);
	ti->Aref = L/(ti->Ta - ti->Ts)/ti->To;
	ti->v1 = (ti->Aref/2.0) * (ti->Ts - sin(2.0 * ti->k * ti->Ts)/(2.0*ti->k) );
	ti->v2 = ti->Aref * (ti->Ta - 2.0*ti->Ts) + ti->v1;
	ti->v3 = ti->v1 + ti->v2;

	ti->t1 = ti->Ts;
	ti->t2 = ti->Ta - ti->Ts;
	ti->t3 = ti->Ta;
	ti->t4 = ti->To;
	ti->t5 = ti->To + ti->Ts;
	ti->t6 = ti->To + ti->Ta - ti->Ts;
	ti->t7 = ti->To + ti->Ta;
	
	M_PlotLabelSetText(ti->plVel, ti->plblCase,  "case %s", which);
	M_PlotLabelReplace(ti->plVel, M_LABEL_X, (Uint)(ti->Ts*L / ti->t7), 0, "Ts");
	M_PlotLabelReplace(ti->plVel, M_LABEL_X, (Uint)(ti->Ta*L / ti->t7), 0, "Ta");
	M_PlotLabelReplace(ti->plVel, M_LABEL_X, (Uint)(ti->To*L / ti->t7), 0, "To");
}

static M_Real
SquaredSineStep(MyTestInstance *ti, M_Real t)
{
	const M_Real Aref = ti->Aref;
	const M_Real k = ti->k;

	if (t <= ti->t1) {
		ti->v = (Aref/2.0)*( t - sin(2.0*k*t)/(2.0*k) );
	} else if (t <= ti->t2) {
		ti->v = Aref*(t - ti->t1) + ti->v1;
	} else if (t <= ti->t3) {
		ti->v = (Aref/2.0)*( (t - ti->t2) +
		                     sin(2.0*k*(t - ti->t2))/(2.0*k) ) + ti->v2;
	} else if (t <= ti->t4) {
		ti->v = ti->v3;
	} else if (t <= ti->t5) {
		ti->v = -(Aref/2.0)*( (t - ti->t4) -
		                      sin(2.0*k*(t - ti->t4))/(2.0*k) ) + ti->v3;
	} else if (t <= ti->t6) {
		ti->v = -Aref*(t - ti->t5) + ti->v2;
	} else if (t <= ti->t7) {
		ti->v = -(Aref/2.0)*( (t - ti->t6) +
		                      sin(2.0*k*(t - ti->t6))/(2.0*k) ) + ti->v1;
	} else {
		ti->v = 0.0;
	}
	return (ti->v);
}

static void
GeneratePlot(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	M_Plotter *plt = M_PLOTTER_PTR(2);
	M_Real t;

	/* Clear the current plot data. */
	M_PlotClear(ti->plVel);
	M_PlotClear(ti->plAcc);
	M_PlotClear(ti->plJerk);

	/*
	 * Compute the data. M_PlotterUpdate() will compute the derivatives
	 * for us.
	 */
	ComputeSquaredSineConstants(ti);
	for (t = 0.0; t < ti->L; t += 1.0) {
		M_PlotReal(ti->plVel, SquaredSineStep(ti, t * ti->t7 / ti->L));
		M_PlotterUpdate(plt);
	}
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->L = 480.0;
	ti->F = 1.0;
	ti->Amax = 0.007;
	ti->Jmax = 0.0001;
	ti->uTs = 0.0;
	ti->uTa = 0.0;
	ti->uTo = 0.0;

	M_InitSubsystem();
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	M_Plotter *plt;
	AG_Pane *pane;
	AG_Numerical *num;
	AG_Box *box;
	int i;

	pane = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	{
		/* Create our plotter widget */
		plt = M_PlotterNew(pane->div[1], M_PLOTTER_EXPAND);

		/*
		 * Create the velocity plot item. This is what our algorithm
		 * computes.
		 */
		ti->plVel = M_PlotNew(plt, M_PLOT_LINEAR);
		M_PlotSetLabel(ti->plVel, "m/s");
		M_PlotSetYoffs(ti->plVel, -45);
		M_PlotSetScale(ti->plVel, 0.0, 50.0);

		/* Create a label we will use to show the "case". */
		ti->plblCase = M_PlotLabelNew(ti->plVel, M_LABEL_OVERLAY, 0, 16, "-");

		/* Plot the derivative of the velocity (the acceleration). */
		ti->plAcc = M_PlotFromDerivative(plt, M_PLOT_LINEAR, ti->plVel);
		M_PlotSetLabel(ti->plAcc, "m/s^2");
		M_PlotSetScale(ti->plAcc, 0.0, 3000.0);

		/* Plot the derivative of the acceleration (the jerk). */
		ti->plJerk = M_PlotFromDerivative(plt, M_PLOT_LINEAR, ti->plAcc);
		M_PlotSetLabel(ti->plJerk, "m/s^3");
		M_PlotSetScale(ti->plJerk, 0.0, 100.0);
		M_PlotSetScale(ti->plJerk, 0.0, 150000.0);
		M_PlotSetYoffs(ti->plJerk, 70);
	}

	/* Allow the user to play with the parameters. */
	box = AG_BoxNew(pane->div[0], AG_BOX_VERT, AG_BOX_EXPAND);
	{
		struct {
			const char *name;
			M_Real *f;
			double incr;
		} param[7] = {
			{ "Jmax: ",	&ti->Jmax,	0.00001 },
			{ "Amax: ",	&ti->Amax,	0.0005 },
			{ "F: ",	&ti->F,		0.01 },
			{ "L: ",	&ti->L,		10.0 },
			{ "Ts tweak: ",	&ti->uTs,	1.0 },
			{ "Ta tweak: ",	&ti->uTa,	1.0 },
			{ "To tweak: ",	&ti->uTo,	1.0 },
		};

		for (i = 0; i < 7; i++) {
			num = AG_NumericalNewS(box, AG_NUMERICAL_HFILL, NULL,
			    param[i].name);
			AG_BindDouble(num, "value", param[i].f);
			AG_SetDouble(num, "inc", param[i].incr);
			AG_SetEvent(num, "numerical-changed",
			    GeneratePlot, "%p,%p", ti, plt);
		}

		AG_SeparatorNewHoriz(box);
		AG_LabelNewPolled(box, AG_LABEL_HFILL, "Aref: %lf", &ti->Aref);
		AG_LabelNewPolled(box, AG_LABEL_HFILL, "v1: %lf", &ti->v1);
		AG_LabelNewPolled(box, AG_LABEL_HFILL, "v2: %lf", &ti->v2);
		AG_LabelNewPolled(box, AG_LABEL_HFILL, "v3: %lf", &ti->v3);
		AG_LabelNewPolled(box, AG_LABEL_HFILL, "Ts: %lf", &ti->Ts);
		AG_LabelNewPolled(box, AG_LABEL_HFILL, "Ta: %lf", &ti->Ta);
		AG_LabelNewPolled(box, AG_LABEL_HFILL, "To: %lf", &ti->To);
		AG_ButtonNewFn(box, AG_BUTTON_HFILL, "Generate",
		    GeneratePlot, "%p,%p", ti, plt);
	}
	AG_AddEvent(win, "window-shown", GeneratePlot, "%p,%p", ti, plt);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 50, 30);
	return (0);
}

const AG_TestCase plottingTest = {
	AGSI_IDEOGRAM AGSI_SINE_WAVE AGSI_RST,
	"plotting",
	N_("Test the M_Plotter(3) widget"),
	"1.6.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
