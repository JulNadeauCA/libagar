/*
 * Copyright (c) 2007-2010 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Default theme for the GUI.
 */

#include <core/core.h>

#include "widget.h"
#include "window.h"
#include "primitive.h"
#include "button.h"
#include "titlebar.h"
#include "checkbox.h"
#include "menu.h"
#include "console.h"
#include "tlist.h"
#include "table.h"
#include "treetbl.h"
#include "textbox.h"
#include "socket.h"
#include "separator.h"
#include "scrollbar.h"
#include "fixed_plotter.h"
#include "graph.h"
#include "notebook.h"
#include "pane.h"
#include "radio.h"
#include "slider.h"

AG_Style agStyleDefault;

/* Background and borders for windows. */
static void
Window(AG_Window *win)
{
	AG_Rect r;
	int hBar = (win->tbar != NULL) ? HEIGHT(win->tbar) : 0;
	AG_Color *bgColor = &agColors[WINDOW_BG_COLOR];

	if (!(win->flags & AG_WINDOW_NOBACKGROUND)) {
		AG_DrawRect(win,
		    AG_RECT(0, hBar-1, WIDTH(win), HEIGHT(win)-hBar),
		    *bgColor);
	}
	if (win->wBorderBot > 0) {
		r.x = 0;
		r.y = HEIGHT(win) - win->wBorderBot;
		r.h = win->wBorderBot;

		if (!(win->flags & AG_WINDOW_NORESIZE) &&
		    WIDTH(win) > win->wResizeCtrl*2) {
			r.w = win->wResizeCtrl;
			AG_DrawBox(win, r,
			    AG_WindowSelectedWM(win,AG_WINOP_LRESIZE) ? -1 : 1,
			    agColors[WINDOW_BORDER_COLOR]);

			r.x = WIDTH(win) - win->wResizeCtrl;
			AG_DrawBox(win, r,
			    AG_WindowSelectedWM(win,AG_WINOP_RRESIZE) ? -1 : 1,
			    agColors[WINDOW_BORDER_COLOR]);
			
			r.x = win->wResizeCtrl;
			r.w = WIDTH(win) - win->wResizeCtrl*2;
			AG_DrawBox(win, r,
			    AG_WindowSelectedWM(win,AG_WINOP_HRESIZE) ? -1 : 1,
			    agColors[WINDOW_BORDER_COLOR]);
		} else {
			r.w = WIDTH(win);
			AG_DrawBox(win, r, 1, agColors[WINDOW_BORDER_COLOR]);
		}
	}
	if (win->wBorderSide > 0) {
		r.x = 0;
		r.y = hBar;
		r.w = win->wBorderSide;
		r.h = HEIGHT(win) - win->wBorderBot - hBar;
		AG_DrawBox(win, r, 1, agColors[WINDOW_BORDER_COLOR]);
		r.x = WIDTH(win) - win->wBorderSide;
		AG_DrawBox(win, r, 1, agColors[WINDOW_BORDER_COLOR]);
	}
}

/* Background of default Titlebar widgets */
static void
TitlebarBackground(void *tbar, int isPressed, int windowIsFocused)
{
	AG_DrawBox(tbar,
	    AG_RECT(0, 0, WIDTH(tbar), HEIGHT(tbar)),
	    isPressed ? -1 : 1,
	    windowIsFocused ? agColors[TITLEBAR_FOCUSED_COLOR] :
	                      agColors[TITLEBAR_UNFOCUSED_COLOR]);
}

/* Background for Button widgets */
static void
ButtonBackground(void *btn, int isPressed)
{
	if (AG_WidgetEnabled(btn)) {
		AG_DrawBox(btn,
		    AG_RECT(0, 0, WIDTH(btn), HEIGHT(btn)),
		    isPressed ? -1 : 1,
		    agColors[BUTTON_COLOR]);
	} else {
		AG_DrawBoxDisabled(btn,
		    AG_RECT(0, 0, WIDTH(btn), HEIGHT(btn)),
		    isPressed ? -1 : 1,
		    agColors[BUTTON_COLOR],
		    agColors[DISABLED_COLOR]);
	}
}

/* Offset for Button labels */
static void
ButtonTextOffset(void *btn, int state, int *x, int *y)
{
	if (state) {
		(*x)++;
		(*y)++;
	}
}

/* Background for Box-style containers */
static void
BoxFrame(void *box, AG_Rect r, int depth)
{
	AG_DrawBox(box, r, depth, agColors[FRAME_COLOR]);
}

/* Button for Checkbox widgets */
static void
CheckboxButton(void *cbox, int state, int size)
{
	if (AG_WidgetEnabled(cbox)) {
		AG_DrawBox(cbox,
		    AG_RECT(0, 0, size, size),
		    state ? -1 : 1,
		    agColors[CHECKBOX_COLOR]);
	} else {
		AG_DrawBoxDisabled(cbox,
		    AG_RECT(0, 0, size, size),
		    state ? -1 : 1,
		    agColors[CHECKBOX_COLOR],
		    agColors[DISABLED_COLOR]);
	}
}

/* Background for Console widgets */
static void
ConsoleBackground(void *cons, AG_Color bgColor)
{
	AG_DrawRect(cons,
	    AG_RECT(0, 0, WIDTH(cons), HEIGHT(cons)),
	    bgColor);
}

/* Background and x-axis line for FixedPlotter widgets */
static void
FixedPlotterBackground(void *fpl, int showAxis, Uint32 yOffset)
{
	AG_DrawBox(fpl,
	    AG_RECT(0, 0, WIDTH(fpl), HEIGHT(fpl)), 0,
	    agColors[GRAPH_BG_COLOR]);
	if (showAxis) {
		AG_DrawLineH(fpl, 0, WIDTH(fpl)-1, yOffset+1,
		    agColors[GRAPH_XAXIS_COLOR]);
	}
}

/* Background for root Menu widgets */
static void
MenuRootBackground(void *m)
{
	AG_DrawBox(m,
	    AG_RECT(0, 0, WIDTH(m), HEIGHT(m)), 1,
	    agColors[MENU_UNSEL_COLOR]);
}

/* Background decorations for selected root Menu items */
static void
MenuRootSelectedItemBackground(void *m, AG_Rect r)
{
	AG_DrawRect(m, r, agColors[MENU_SEL_COLOR]);
}

/* Background for Menu views */
static void
MenuBackground(void *mv, AG_Rect r)
{
	AG_DrawBox(mv, r, 1, agColors[MENU_UNSEL_COLOR]);
}

/* Menu separator item */
static void
MenuItemSeparator(void *mv, int x1, int x2, int y, int h)
{
	AG_DrawLineH(mv, x1, x2, (y + h/2 - 1), agColors[MENU_SEP1_COLOR]);
	AG_DrawLineH(mv, x1, x2, (y + h/2), agColors[MENU_SEP2_COLOR]);
}

/* Background for Notebook widget */
static void
NotebookBackground(void *nb, AG_Rect r)
{
	/* XXX use something less expensive */
	AG_DrawRect(nb, r, agColors[NOTEBOOK_SEL_COLOR]);
}

/* Background for individual notebook tab */
static void
NotebookTabBackground(void *nb, AG_Rect r, int idx, int isSelected)
{
	AG_DrawBoxRoundedTop(nb, r,
	    isSelected ? -1 : 1, (int)(agTextFontHeight/1.5),
	    isSelected ? agColors[NOTEBOOK_SEL_COLOR] :
	                 agColors[NOTEBOOK_BG_COLOR]);
}

/* Horizontal divider for Pane widget */
static void
PaneHorizDivider(void *pa, int x, int y, int w, int isMoving)
{
	int xMid = x + w/2;
	AG_Color c;

	AG_DrawBox(pa,
	    AG_RECT(x+1, 0, w-2, HEIGHT(pa)),
	    isMoving?-1:1,
	    agColors[PANE_COLOR]);
	
	c = agColors[PANE_CIRCLE_COLOR];
	AG_PutPixel(pa, xMid, y, c);
	AG_PutPixel(pa, xMid, y-5, c);
	AG_PutPixel(pa, xMid, y+5, c);
}

/* Vertical divider for Pane widget */
static void
PaneVertDivider(void *pa, int x, int y, int w, int isMoving)
{
	int yMid = y + w/2;
	AG_Color c;

	AG_DrawBox(pa,
	    AG_RECT(0, y+1, WIDTH(pa), w-2),
	    isMoving?-1:1,
	    agColors[PANE_COLOR]);

	c = agColors[PANE_CIRCLE_COLOR];
	AG_PutPixel(pa, x, yMid, c);
	AG_PutPixel(pa, x-5, yMid, c);
	AG_PutPixel(pa, x+5, yMid, c);
}

/* Background for Radio group */
static void
RadioGroupBackground(void *rad, AG_Rect r)
{
	AG_DrawFrame(rad, r, AG_WidgetIsFocusedInWindow(rad) ? -1 : 1,
	    agColors[FRAME_COLOR]);
}

/* Radio button */
static void
RadioButton(AG_Radio *rad, int x, int y, int selected, int over)
{
	static const int highlight[18] = {
		-5, +1,	-5,  0,	-5, -1,	-4, -2,	-4, -3,	-3, -4,	-2, -4,
		 0, -5,	-1, -5
	};
	int xc = rad->xPadding + rad->radius;
	int yc = y + rad->radius;
	int i;
	AG_Color cHi = agColors[RADIO_HI_COLOR];
	AG_Color cLo = agColors[RADIO_LO_COLOR];

	for (i = 0; i < 18; i+=2) {
		AG_PutPixel(rad,
		    xc + highlight[i],
		    yc + highlight[i+1],
		    cHi);
		AG_PutPixel(rad,
		    xc - highlight[i],
		    yc - highlight[i+1],
		    cLo);
	}

	if (selected) {
		AG_DrawCircle(rad,
		    rad->xPadding + rad->radius,
		    y + rad->radius,
		    rad->radius/2,
		    agColors[RADIO_SEL_COLOR]);
	} else if (over) {
		AG_DrawCircle(rad,
		    rad->xPadding + rad->radius,
		    y + rad->radius,
		    rad->radius/2,
		    agColors[RADIO_OVER_COLOR]);
	}
}

/* Background for ProgressBar */
static void
ProgressBarBackground(void *pb)
{
	AG_DrawBox(pb,
	    AG_RECT(0, 0, WIDTH(pb), HEIGHT(pb)), -1,
	    agColors[FRAME_COLOR]);
}

/* Background for horizontal Slider */
static void
SliderBackgroundHoriz(void *sl)
{
	int h = HEIGHT(sl)/3;

	AG_DrawBox(sl,
	    AG_RECT(0, HEIGHT(sl)/2 - h/2, WIDTH(sl), h), -1,
	    agColors[SCROLLBAR_COLOR]);
}

/* Background for vertical Slider */
static void
SliderBackgroundVert(void *sl)
{
	int w = WIDTH(sl)/3;

	AG_DrawBox(sl,
	    AG_RECT(WIDTH(sl)/2 - w/2, 0, w, HEIGHT(sl)), -1,
	    agColors[SCROLLBAR_COLOR]);
}

/* Control for horizontal Slider */
static void
SliderControlHoriz(void *p, int x, int w)
{
	AG_Slider *sl = p;
	AG_DrawBox(sl,
	    AG_RECT(x, 0, w, HEIGHT(sl)),
	    sl->ctlPressed ? -1 : 1,
	    agColors[SCROLLBAR_BTN_COLOR]);
}

/* Control for vertical Slider */
static void
SliderControlVert(void *p, int y, int h)
{
	AG_Slider *sl = p;
	AG_DrawBox(sl,
	    AG_RECT(0, y, WIDTH(sl), h),
	    sl->ctlPressed ? -1 : 1,
	    agColors[SCROLLBAR_BTN_COLOR]);
}

/* Vertical scrollbar (undersize case). */
static void
ScrollbarVertUndersize(AG_Scrollbar *sb)
{
	int w = WIDTH(sb)/2;
	int size = MIN(HEIGHT(sb)/4, WIDTH(sb));

	AG_DrawBox(sb, AG_RECT(0,0,WIDTH(sb),HEIGHT(sb)), 1,
	    agColors[SCROLLBAR_BTN_COLOR]);
	AG_DrawArrowUp(sb,
	    w,
	    size,
	    size,
	    agColors[SCROLLBAR_ARR1_COLOR],
	    agColors[SCROLLBAR_ARR2_COLOR]);
	AG_DrawArrowDown(sb,
	    w,
	    HEIGHT(sb)/2 + size,
	    size,
	    agColors[SCROLLBAR_ARR1_COLOR],
	    agColors[SCROLLBAR_ARR2_COLOR]);
}

/* Buttons of vertical Scrollbar */
static void
ScrollbarVert(AG_Scrollbar *sb, int y, int h)
{
	int mid = WIDTH(sb)/2;
	int b2 = sb->width*2;
	int hArrow = MIN(WIDTH(sb), sb->hArrow);
	int y2;

	if (HEIGHT(sb) < b2) {
		ScrollbarVertUndersize(sb);
		return;
	}

	/* Background */
	AG_DrawBox(sb,
	    AG_RECT(0, 0, WIDTH(sb), HEIGHT(sb)), -1,
	    agColors[SCROLLBAR_COLOR]);

	/* Upper button. */
	AG_DrawBox(sb,
	    AG_RECT(0, 0, WIDTH(sb), sb->width),
	    (sb->curBtn == AG_SCROLLBAR_BUTTON_DEC) ? -1 : 1,
	    agColors[SCROLLBAR_BTN_COLOR]);
	AG_DrawArrowUp(sb,
	    mid,
	    sb->width/2,
	    hArrow,
	    agColors[SCROLLBAR_ARR1_COLOR],
	    agColors[SCROLLBAR_ARR2_COLOR]);
	
	/* Lower button. */
	y2 = HEIGHT(sb) - sb->width;
	AG_DrawBox(sb,
	    AG_RECT(0, y2, WIDTH(sb), sb->width),
	    (sb->curBtn == AG_SCROLLBAR_BUTTON_INC) ? -1 : 1,
	    agColors[SCROLLBAR_BTN_COLOR]);
	AG_DrawArrowDown(sb,
	    mid,
	    y2 + sb->width/2,
	    hArrow,
	    agColors[SCROLLBAR_ARR1_COLOR],
	    agColors[SCROLLBAR_ARR2_COLOR]);

	/* Scrollbar. */
	if (h > 0) {
		AG_DrawBox(sb,
		    AG_RECT(0,
		            sb->width + y,
			    WIDTH(sb),
			    MIN(h, HEIGHT(sb)-b2)),
		    (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) ? -1 : 1,
		    agColors[SCROLLBAR_BTN_COLOR]);
	} else {
		AG_DrawBox(sb,
		    AG_RECT(0,
		            sb->width,
			    WIDTH(sb),
		            HEIGHT(sb)-b2),
		    (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) ? -1 : 1,
		    agColors[SCROLLBAR_BTN_COLOR]);
	}
}

/* Horizontal scrollbar (undersize case). */
static void
ScrollbarHorizUndersize(AG_Scrollbar *sb)
{
	int h = HEIGHT(sb)/2;
	int size = MIN(WIDTH(sb)/4, HEIGHT(sb));

	AG_DrawBox(sb, AG_RECT(0,0,WIDTH(sb),HEIGHT(sb)), 1,
	    agColors[SCROLLBAR_BTN_COLOR]);
	AG_DrawArrowLeft(sb,
	    size,
	    h,
	    size,
	    agColors[SCROLLBAR_ARR1_COLOR],
	    agColors[SCROLLBAR_ARR2_COLOR]);
	AG_DrawArrowRight(sb,
	    WIDTH(sb)/2 + size,
	    h,
	    size,
	    agColors[SCROLLBAR_ARR1_COLOR],
	    agColors[SCROLLBAR_ARR2_COLOR]);
}

/* Buttons of horizontal Scrollbar */
static void
ScrollbarHoriz(AG_Scrollbar *sb, int x, int w)
{
	int mid = HEIGHT(sb)/2;
	int b2 = sb->width*2;
	int hArrow = MIN(HEIGHT(sb), sb->hArrow);
	int x2;
	
	if (WIDTH(sb) < b2) {
		ScrollbarHorizUndersize(sb);
		return;
	}
	
	/* Background */
	AG_DrawBox(sb,
	    AG_RECT(0, 0, WIDTH(sb), HEIGHT(sb)), -1,
	    agColors[SCROLLBAR_COLOR]);

	/* Left button */
	AG_DrawBox(sb,
	    AG_RECT(0, 0, sb->width, HEIGHT(sb)),
	    (sb->curBtn == AG_SCROLLBAR_BUTTON_DEC) ? -1 : 1,
	    agColors[SCROLLBAR_BTN_COLOR]);
	AG_DrawArrowLeft(sb,
	    sb->width/2, mid,
	    hArrow,
	    agColors[SCROLLBAR_ARR1_COLOR],
	    agColors[SCROLLBAR_ARR2_COLOR]);
	
	/* Right button */
	x2 = WIDTH(sb) - sb->width;
	AG_DrawBox(sb,
	    AG_RECT(x2, 0, sb->width, HEIGHT(sb)),
	    (sb->curBtn == AG_SCROLLBAR_BUTTON_INC) ? -1 : 1,
	    agColors[SCROLLBAR_BTN_COLOR]);
	AG_DrawArrowRight(sb, (x2 + sb->width/2), mid, hArrow,
	    agColors[SCROLLBAR_ARR1_COLOR],
	    agColors[SCROLLBAR_ARR2_COLOR]);

	/* Scrollbar */
	if (w > 0) {
		AG_DrawBox(sb,
		    AG_RECT(sb->width + x,
		            0,
			    MIN(w, WIDTH(sb)-b2),
			    HEIGHT(sb)),
		    (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) ? -1 : 1,
		    agColors[SCROLLBAR_BTN_COLOR]);
	} else {
		AG_DrawBox(sb,
		    AG_RECT(sb->width,
		            0,
			    WIDTH(sb)-b2,
		            HEIGHT(sb)),
		    (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) ? -1 : 1,
		    agColors[SCROLLBAR_BTN_COLOR]);
	}
}

/* Horizontal Separator */
static void
SeparatorHoriz(AG_Separator *sep)
{
	AG_DrawLineH(sep, 0, WIDTH(sep), sep->padding,
	    agColors[SEPARATOR_LINE1_COLOR]);
	AG_DrawLineH(sep, 0, WIDTH(sep), sep->padding+1,
	    agColors[SEPARATOR_LINE2_COLOR]);
}

/* Vertical Separator */
static void
SeparatorVert(AG_Separator *sep)
{
	AG_DrawLineV(sep, sep->padding, 0, HEIGHT(sep),
	    agColors[SEPARATOR_LINE1_COLOR]);
	AG_DrawLineV(sep, sep->padding+1, 0, HEIGHT(sep),
	    agColors[SEPARATOR_LINE2_COLOR]);
}

/* Background for Socket widgets */
static void
SocketBackground(AG_Socket *sock)
{
	switch (sock->bgType) {
	case AG_SOCKET_PIXMAP:
		AG_WidgetBlitSurface(sock, sock->bgData.pixmap.s, 0, 0);
		break;
	case AG_SOCKET_RECT:
		if (AG_WidgetEnabled(sock)) {
			AG_DrawBox(sock,
			    AG_RECT(0, 0, WIDTH(sock), HEIGHT(sock)), -1,
			    agColors[SOCKET_COLOR]);
		} else {
			AG_DrawBoxDisabled(sock,
			    AG_RECT(0, 0, WIDTH(sock), HEIGHT(sock)), -1,
			    agColors[SOCKET_COLOR],
			    agColors[DISABLED_COLOR]);
		}
		break;
	case AG_SOCKET_CIRCLE:
		AG_DrawCircle(sock, WIDTH(sock)/2, HEIGHT(sock)/2,
		    sock->bgData.circle.r, agColors[SOCKET_COLOR]);
		break;
	}
}

/* Overlay for Socket widgets */
static void
SocketOverlay(AG_Socket *sock, int highlight)
{
	switch (sock->bgType) {
	case AG_SOCKET_PIXMAP:
		/* TODO */
	case AG_SOCKET_RECT:
		if (highlight) {
			AG_DrawRectOutline(sock,
			    AG_RECT(sock->lPad, sock->tPad,
			            WIDTH(sock) - sock->lPad - sock->rPad,
				    HEIGHT(sock) - sock->tPad - sock->bPad),
			    agColors[SOCKET_HIGHLIGHT_COLOR]);
		}
		break;
	case AG_SOCKET_CIRCLE:
		if (highlight) {
			AG_DrawCircle(sock, WIDTH(sock)/2, HEIGHT(sock)/2,
			    sock->bgData.circle.r - sock->lPad,
			    agColors[SOCKET_HIGHLIGHT_COLOR]);
		}
		break;
	}
}

/* Background for table widgets */
static void
TableBackground(void *tbl, AG_Rect r)
{
	AG_DrawBox(tbl, r, -1, agColors[TABLE_COLOR]);
}

/* Background for table column headers */
static void
TableColumnHeaderBackground(void *tbl, int idx, AG_Rect r, int isSelected)
{
	AG_DrawBox(tbl, r, isSelected?-1:1, agColors[TABLE_COLOR]);
}

/* Background for selected table columns */
static void
TableSelectedColumnBackground(void *tbl, int idx, AG_Rect r)
{
	AG_DrawRectBlended(tbl, r,
	    AG_ColorRGBA(0,0,250,32),
	    AG_ALPHA_SRC);
}

/* Background for table rows */
static void
TableRowBackground(void *wid, AG_Rect r, int isSelected)
{
	if (isSelected)
		AG_DrawBox(wid, r, 1, agColors[TABLEVIEW_SEL_COLOR]);
}

/* Background for textbox widgets */
static void
TextboxBackground(void *tbox, AG_Rect r, int isCombo)
{
	if (AG_WidgetDisabled(tbox)) {
		AG_DrawBoxDisabled(tbox, r, -1,
		    agColors[TEXTBOX_COLOR],
		    agColors[DISABLED_COLOR]);
	} else {
		AG_DrawBox(tbox, r, isCombo?1:-1,
		    agColors[TEXTBOX_COLOR]);
	}
}

/* Background of list widgets */
static void
ListBackground(void *tl, AG_Rect r)
{
	AG_DrawBox(tl, r, -1, agColors[TLIST_BG_COLOR]);
}

/* Background for selected Tlist items */
static void
ListItemBackground(void *tl, AG_Rect r, int isSelected)
{
	if (isSelected)
		AG_DrawRect(tl, r, agColors[TLIST_SEL_COLOR]);
}

/* Indicate presence of child items in a tree display. */
static void
TreeSubnodeIndicator(void *wid, AG_Rect r, int isExpanded)
{
	AG_Color C;

	AG_DrawRectBlended(wid,
	    AG_RECT(r.x-1, r.y, r.w+2, r.h),
	    AG_ColorRGBA(0,0,0,64),
	    AG_ALPHA_SRC);

	C = AG_ColorRGBA(255,255,255,100);
	if (isExpanded) {
		AG_DrawMinus(wid,
		    AG_RECT(r.x+2, r.y+2, r.w-4, r.h-4),
		    C, AG_ALPHA_SRC);
	} else {
		AG_DrawPlus(wid,
		    AG_RECT(r.x+2, r.y+2, r.w-4, r.h-4),
		    C, AG_ALPHA_SRC);
	}
}

/*
 * Note: The layout of AG_Style may change, so if you are writing a theme,
 * do not use a static initializer like this - override the individual
 * members instead.
 */
AG_Style agStyleDefault = {
	"default",
	{ 0, 0 },
	NULL,			/* init */
	NULL,			/* destroy */
	Window,
	TitlebarBackground,
	ButtonBackground,
	ButtonTextOffset,
	BoxFrame,
	CheckboxButton,
	ConsoleBackground,
	FixedPlotterBackground,
	MenuRootBackground,
	MenuRootSelectedItemBackground,
	MenuBackground,
	NULL,					/* MenuItemBackground (OBSOL) */
	MenuItemSeparator,
	NotebookBackground,
	NotebookTabBackground,
	PaneHorizDivider,
	PaneVertDivider,
	RadioGroupBackground,
	RadioButton,
	ProgressBarBackground,
	ScrollbarVert,
	ScrollbarHoriz,
	SliderBackgroundHoriz,
	SliderBackgroundVert,
	SliderControlHoriz,
	SliderControlVert,
	SeparatorHoriz,
	SeparatorVert,
	SocketBackground,
	SocketOverlay,
	TableBackground,
	TableColumnHeaderBackground,
	TableSelectedColumnBackground,
	TableRowBackground,
	TextboxBackground,
	ListBackground,
	ListItemBackground,
	TreeSubnodeIndicator
};
