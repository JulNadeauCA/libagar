/*	Public domain	*/

/*
 * An example widget for drawing bezier curves.
 */

#include <stdlib.h>

#include "agartest.h"
#include "bezier_widget.h"

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

MY_BezierWidget *
MY_BezierWidgetNew(void *parent, const char *foo)
{
	MY_BezierWidget *bez;

	bez = AG_Malloc(sizeof(MY_BezierWidget));
	AG_ObjectInit(bez, &myBezierWidgetClass);
	AG_ObjectAttach(parent, bez);
	return (bez);
}

static void
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
{
	MY_BezierWidget *bez = p;

	if (bez->label == -1) {
		AG_TextSize("<M_Bezier2 test>", &r->w, &r->h);
	} else {
		r->w = AGWIDGET_SURFACE(bez,bez->label)->w;
		r->h = AGWIDGET_SURFACE(bez,bez->label)->h;
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
	MY_BezierWidget *bez = p;
	AG_Color c;
	AG_Rect r;

	/* Draw a raised 3D box over the widget area; use background-color. */
	r.x = 0;
	r.y = 0;
	r.w = AGWIDGET(bez)->w;
	r.h = AGWIDGET(bez)->h;
	AG_DrawBoxRaised(bez, &r, &AG_WCOLOR(bez, AG_BG_COLOR));

	if (bez->label == -1) {
		AG_TextFontPct(500);
		bez->label = AG_WidgetMapSurface(bez,
		    AG_TextRender("M_Bezier2 test"));
	}

	AG_ColorRGB_8(&c, 250,250,0);

	AG_PushClipRect(bez, &r);

	/* TODO */

	M_DrawBezier2(AGWIDGET(bez), 100,20,  120,40,  140,25,  160,20,  20, &c);
	M_DrawBezier2(AGWIDGET(bez), 200,200, 120,40,  140,300, 160,20,  20, &c);
	M_DrawBezier2(AGWIDGET(bez), 100,200, 120,200, 40,300,  160,20,  20, &c);
	M_DrawBezier2(AGWIDGET(bez), 30,10,   43,34,   40,134,  160,212, 5,  &c);

	AG_PopClipRect(bez);
}

static void
Init(void *_Nonnull obj)
{
	MY_BezierWidget *bez = obj;

	AGWIDGET(bez)->flags |= (AG_WIDGET_FOCUSABLE | AG_WIDGET_UNFOCUSED_MOTION |
	                         AG_WIDGET_USE_TEXT | AG_WIDGET_USE_MOUSEOVER);

	bez->label = -1;
}

AG_WidgetClass myBezierWidgetClass = {
	{
		"AG_Widget:MY_BezierWidget",
		sizeof(MY_BezierWidget),
		{ 0,0, AGC_WIDGET, 0 },
		Init,
		NULL,    /* reset */
		NULL,    /* destroy */
		NULL,    /* load */
		NULL,    /* save */
		NULL     /* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate,
	NULL,            /* mouse_button_down */
	NULL,            /* mouse_button_up */
	NULL,            /* mouse_motion */
	NULL,            /* key_down */
	NULL,            /* key_up */
	NULL,            /* touch */
	NULL,            /* ctrl */
	NULL             /* joy */
};
