/*	Public domain	*/

/*
 * Test for M_DrawBezier().
 */

#include <stdlib.h>

#include "agartest.h"
#include "bezier_widget.h"

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

BezierWidget *
BezierWidgetNew(void *parent, const char *foo)
{
	BezierWidget *my;

	my = AG_Malloc(sizeof(BezierWidget));
	AG_ObjectInit(my, &bezierWidgetClass);
	AG_ObjectAttach(parent, my);
	return (my);
}

static void
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
{
	BezierWidget *my = p;

	if (my->label == -1) {
		AG_TextSize("<M_Bezier2 test>", &r->w, &r->h);
	} else {
		r->w = AGWIDGET_SURFACE(my,my->label)->w;
		r->h = AGWIDGET_SURFACE(my,my->label)->h;
	}
}

static int
SizeAllocate(void *_Nonnull p, const AG_SizeAlloc *_Nonnull a)
{
	if (a->w < 5 || a->h < 5)
		return (-1);

	return (0);
}

static void
Draw(void *_Nonnull p)
{
	BezierWidget *my = p;
	AG_Color c;
	AG_Rect r;

	/* Draw a raised 3D box over the widget area; use background-color. */
	r.x = 0;
	r.y = 0;
	r.w = AGWIDGET(my)->w;
	r.h = AGWIDGET(my)->h;
	AG_DrawBoxRaised(my, &r, &AG_WCOLOR(my, AG_BG_COLOR));

	if (my->label == -1) {
		AG_TextFontPct(500);
		my->label = AG_WidgetMapSurface(my,
		    AG_TextRender("M_Bezier2 test"));
	}

	AG_ColorRGB_8(&c, 250,250,0);

	AG_PushClipRect(my, &r);

	M_DrawBezier2(AGWIDGET(my), 100,20,  120,40,  140,25,  160,20,  20, &c);
	M_DrawBezier2(AGWIDGET(my), 200,200, 120,40,  140,300, 160,20,  20, &c);
	M_DrawBezier2(AGWIDGET(my), 100,200, 120,200, 40,300,  160,20,  20, &c);
	M_DrawBezier2(AGWIDGET(my), 30,10,   43,34,   40,134,  160,212, 5,  &c);

	AG_PopClipRect(my);
}


/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of the parent classes first.
 */
static void
Init(void *_Nonnull obj)
{
	BezierWidget *my = obj;

	AGWIDGET(my)->flags |= (AG_WIDGET_FOCUSABLE | AG_WIDGET_UNFOCUSED_MOTION |
	                        AG_WIDGET_USE_TEXT | AG_WIDGET_USE_MOUSEOVER);

	my->foo = "";
	my->x = 0;
	my->y = 0;
	my->radius = 5;
	my->lastKey = 'X';
	my->label = -1;
}

AG_WidgetClass bezierWidgetClass = {
	{
		"AG_Widget:BezierWidget",
		sizeof(BezierWidget),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate,
	NULL,			/* mouse_button_down */
	NULL,			/* mouse_button_up */
	NULL,			/* mouse_motion */
	NULL,			/* key_down */
	NULL,			/* key_up */
	NULL,			/* touch */
	NULL,			/* ctrl */
	NULL			/* joy */
};
