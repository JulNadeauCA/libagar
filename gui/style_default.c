/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
#include "tableview.h"
#include "textbox.h"
#include "socket.h"
#include "separator.h"
#include "scrollbar.h"
#include "fixed_plotter.h"
#include "graph.h"
#include "notebook.h"
#include "pane.h"
#include "radio.h"

AG_Style agStyleDefault;

/* Background for windows that don't have NOBACKGROUND set. */
static void
WindowBackground(AG_Window *win)
{
	AG_DrawRectFilled(win,
	    AG_RECT(0, 0, WIDTH(win), HEIGHT(win)),
	    AG_COLOR(WINDOW_BG_COLOR));
}

/* Decorations for windows that don't have NOBORDERS set. */
static void
WindowBorders(AG_Window *win)
{
	int i;

	for (i = 1; i < agColorsBorderSize-1; i++) {
		AG_DrawLineH(win, i, WIDTH(win)-i, i, agColorsBorder[i-1]);
		AG_DrawLineH(win, i, WIDTH(win)-i, HEIGHT(win)-i,
		    agColorsBorder[i-1]);
	}
	for (i = 1; i < agColorsBorderSize-1; i++) {
		AG_DrawLineV(win, i-1, i, HEIGHT(win)-i, agColorsBorder[i-1]);
		AG_DrawLineV(win, WIDTH(win)-i, i, HEIGHT(win)-i,
		    agColorsBorder[i-1]);
	}
	/* Indicate the resize controls */
	if ((win->flags & AG_WINDOW_NOHRESIZE) == 0) {
		AG_DrawLineV(win, 18, HEIGHT(win)-agColorsBorderSize,
		    HEIGHT(win)-2, AG_COLOR(WINDOW_LO_COLOR));
		AG_DrawLineV(win, 19, HEIGHT(win)-agColorsBorderSize,
		    HEIGHT(win)-2, AG_COLOR(WINDOW_HI_COLOR));
		AG_DrawLineV(win, WIDTH(win)-19, HEIGHT(win)-agColorsBorderSize,
		    HEIGHT(win)-2, AG_COLOR(WINDOW_LO_COLOR));
		AG_DrawLineV(win, WIDTH(win)-18, HEIGHT(win)-agColorsBorderSize,
		    HEIGHT(win)-2, AG_COLOR(WINDOW_HI_COLOR));
	}
	if ((win->flags & AG_WINDOW_NOVRESIZE) == 0) {
		AG_DrawLineH(win, 2, agColorsBorderSize, HEIGHT(win)-20,
		    AG_COLOR(WINDOW_LO_COLOR));
		AG_DrawLineH(win, 2, agColorsBorderSize, HEIGHT(win)-19,
		    AG_COLOR(WINDOW_HI_COLOR));
		AG_DrawLineH(win, WIDTH(win)-agColorsBorderSize,
		    WIDTH(win)-2, HEIGHT(win)-20, AG_COLOR(WINDOW_LO_COLOR));
		AG_DrawLineH(win, WIDTH(win)-agColorsBorderSize,
		    WIDTH(win)-2, HEIGHT(win)-19, AG_COLOR(WINDOW_HI_COLOR));
	}
}

/* Background of default Titlebar widgets */
static void
TitlebarBackground(void *tbar, int isPressed, int windowIsFocused)
{
	AG_DrawBox(tbar,
	    AG_RECT(0, 0, WIDTH(tbar), HEIGHT(tbar)),
	    isPressed ? -1 : 1,
	    windowIsFocused ? AG_COLOR(TITLEBAR_FOCUSED_COLOR) :
	                      AG_COLOR(TITLEBAR_UNFOCUSED_COLOR));
}

/* Background for Button widgets */
static void
ButtonBackground(void *btn, int isPressed)
{
	if (AG_WidgetEnabled(btn)) {
		AG_DrawBox(btn,
		    AG_RECT(0, 0, WIDTH(btn), HEIGHT(btn)),
		    isPressed ? -1 : 1,
		    AG_COLOR(BUTTON_COLOR));
	} else {
		AG_DrawBoxDithered(btn,
		    AG_RECT(0, 0, WIDTH(btn), HEIGHT(btn)),
		    isPressed ? -1 : 1,
		    AG_COLOR(BUTTON_COLOR),
		    AG_COLOR(DISABLED_COLOR));
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
BoxFrame(void *box, int depth)
{
	AG_DrawBox(box,
	    AG_RECT(0, 0, WIDTH(box), HEIGHT(box)), depth,
	    AG_COLOR(FRAME_COLOR));
}

/* Button for Checkbox widgets */
static void
CheckboxButton(void *cbox, int state)
{
	if (AG_WidgetEnabled(cbox)) {
		AG_DrawBox(cbox,
		    AG_RECT(0, 0, HEIGHT(cbox), HEIGHT(cbox)),
		    state ? -1 : 1,
		    AG_COLOR(CHECKBOX_COLOR));
	} else {
		AG_DrawBoxDithered(cbox,
		    AG_RECT(0, 0, HEIGHT(cbox), HEIGHT(cbox)),
		    state ? -1 : 1,
		    AG_COLOR(CHECKBOX_COLOR),
		    AG_COLOR(DISABLED_COLOR));
	}
}

/* Background for Console widgets */
static void
ConsoleBackground(void *cons, Uint32 bgColor)
{
	AG_DrawBox(cons,
	    AG_RECT(0, 0, WIDTH(cons), HEIGHT(cons)), -1,
	    bgColor);
}

/* Background and x-axis line for FixedPlotter widgets */
static void
FixedPlotterBackground(void *fpl, int showAxis, Uint32 yOffset)
{
	AG_DrawBox(fpl,
	    AG_RECT(0, 0, WIDTH(fpl), HEIGHT(fpl)), 0,
	    AG_COLOR(GRAPH_BG_COLOR));
	if (showAxis) {
		AG_DrawLineH(fpl, 0, WIDTH(fpl)-1, yOffset+1,
		    AG_COLOR(GRAPH_XAXIS_COLOR));
	}
}

/* Background for root Menu widgets */
static void
MenuRootBackground(void *m)
{
	AG_DrawBox(m,
	    AG_RECT(0, 0, WIDTH(m), HEIGHT(m)), 1,
	    AG_COLOR(MENU_UNSEL_COLOR));
}

/* Background decorations for selected root Menu items */
static void
MenuRootSelectedItemBackground(void *m, AG_Rect r)
{
	AG_DrawRectFilled(m, r, AG_COLOR(MENU_SEL_COLOR));
}

/* Background for Menu views */
static void
MenuBackground(void *mv, AG_Rect r)
{
	AG_DrawBox(mv, r, 1, AG_COLOR(MENU_UNSEL_COLOR));
}

/* Background decorations for selected Menu items */
static void
MenuItemBackground(void *mv, int x, int y, int h, int xIcon, void *iconObj,
    int icon, int isSelected, int boolState)
{
	if (isSelected) {
		AG_DrawRectFilled(mv,
		    AG_RECT(x+1, y+1, WIDTH(mv), h),
		    AG_COLOR(MENU_SEL_COLOR));
	}
	if (icon != -1) {
		Uint8 c[4];

		AG_WidgetBlitFrom(mv, iconObj, icon, NULL,
		    xIcon,
		    y + (h/2 - WSURFACE(iconObj,icon)->h/2) + 1);

		if (boolState) {
			SDL_GetRGB(AG_COLOR(MENU_OPTION_COLOR), agVideoFmt,
			    &c[0], &c[1], &c[2]);
			c[3] = 64;
			AG_DrawFrame(mv,
			    AG_RECT(x, y+2, h, h-2), 1,
			    AG_COLOR(MENU_OPTION_COLOR));
			AG_DrawRectBlended(mv,
			    AG_RECT(x, y+2, h, h-2),
			    c, AG_ALPHA_SRC);
		}
	}
}

/* Menu separator item */
static void
MenuItemSeparator(void *mv, int x1, int x2, int y, int h)
{
	AG_DrawLineH(mv, x1, x2, (y + h/2 - 1), AG_COLOR(MENU_SEP1_COLOR));
	AG_DrawLineH(mv, x1, x2, (y + h/2), AG_COLOR(MENU_SEP2_COLOR));
}

/* Background for Notebook widget */
static void
NotebookBackground(void *nb, int y1)
{
	AG_DrawRectFilled(nb,
	    AG_RECT(0, y1, WIDTH(nb), HEIGHT(nb)-y1),
	    AG_COLOR(NOTEBOOK_SEL_COLOR));
}

/* Background for individual notebook tab */
static void
NotebookTabBackground(void *nb, AG_Rect r, int idx, int isSelected)
{
	AG_DrawBoxRounded(nb, r,
	    isSelected ? -1 : 1, (int)(agTextFontHeight/1.5),
	    isSelected ? AG_COLOR(NOTEBOOK_SEL_COLOR) :
	                 AG_COLOR(NOTEBOOK_BG_COLOR));
}

/* Horizontal divider for Pane widget */
static void
PaneHorizDivider(void *pa, int x, int y, int w, int isMoving)
{
	AG_DrawBox(pa,
	    AG_RECT(x+1, 0, w-2, HEIGHT(pa)),
	    isMoving?-1:1,
	    AG_COLOR(PANE_COLOR));
	if (!agView->opengl) {
		int cx = x + w/2;
		AG_WidgetPutPixel(pa, cx, y, AG_COLOR(PANE_CIRCLE_COLOR));
		AG_WidgetPutPixel(pa, cx, y-5, AG_COLOR(PANE_CIRCLE_COLOR));
		AG_WidgetPutPixel(pa, cx, y+5, AG_COLOR(PANE_CIRCLE_COLOR));
	}
}

/* Vertical divider for Pane widget */
static void
PaneVertDivider(void *pa, int x, int y, int w, int isMoving)
{
	AG_DrawBox(pa,
	    AG_RECT(0, y+1, WIDTH(pa), w-2),
	    isMoving?-1:1,
	    AG_COLOR(PANE_COLOR));
	if (!agView->opengl) {
		int cy = y + w/2;
		AG_WidgetPutPixel(pa, x, cy, AG_COLOR(PANE_CIRCLE_COLOR));
		AG_WidgetPutPixel(pa, x-5, cy, AG_COLOR(PANE_CIRCLE_COLOR));
		AG_WidgetPutPixel(pa, x+5, cy, AG_COLOR(PANE_CIRCLE_COLOR));
	}
}

/* Background for Radio group */
static void
RadioGroupBackground(void *rad, AG_Rect r)
{
	AG_DrawFrame(rad, r, 1, AG_COLOR(FRAME_COLOR));
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

	for (i = 0; i < 18; i+=2) {
		AG_WidgetPutPixel(rad,
		    xc + highlight[i],
		    yc + highlight[i+1],
		    AG_COLOR(RADIO_HI_COLOR));
		AG_WidgetPutPixel(rad,
		    xc - highlight[i],
		    yc - highlight[i+1],
		    AG_COLOR(RADIO_LO_COLOR));
	}
	if (selected) {
		AG_DrawCircle(rad,
		    rad->xPadding + rad->radius,
		    y + rad->radius,
		    rad->radius/2,
		    AG_COLOR(RADIO_SEL_COLOR));
	} else if (over) {
		AG_DrawCircle(rad,
		    rad->xPadding + rad->radius,
		    y + rad->radius,
		    rad->radius/2,
		    AG_COLOR(RADIO_OVER_COLOR));
	}
}

/* Background for Scrollbar */
static void
ScrollbarBackground(void *sb)
{
	AG_DrawBox(sb,
	    AG_RECT(0, 0, WIDTH(sb), HEIGHT(sb)), -1,
	    AG_COLOR(SCROLLBAR_COLOR));
}

/* Buttons of vertical Scrollbar */
static void
ScrollbarVertButtons(AG_Scrollbar *sb, int y, int h)
{
	int y2;

	AG_DrawBox(sb,
	    AG_RECT(0, 0, sb->bw, sb->bw),
	    (sb->curbutton == 1) ? -1 : 1,
	    AG_COLOR(SCROLLBAR_BTN_COLOR));
	AG_DrawArrowUp(sb, sb->bw/2, sb->bw/2, sb->arrowSz,
	    AG_COLOR(SCROLLBAR_ARR1_COLOR),
	    AG_COLOR(SCROLLBAR_ARR2_COLOR));
	
	y2 = HEIGHT(sb) - sb->bw;
	AG_DrawBox(sb,
	    AG_RECT(0, y2, sb->bw, sb->bw),
	    (sb->curbutton == 2) ? -1 : 1,
	    AG_COLOR(SCROLLBAR_BTN_COLOR));
	AG_DrawArrowDown(sb, sb->bw/2, (y2 + sb->bw/2), sb->arrowSz,
	    AG_COLOR(SCROLLBAR_ARR1_COLOR),
	    AG_COLOR(SCROLLBAR_ARR2_COLOR));
	
	AG_DrawBox(sb,
	    AG_RECT(0, sb->bw+y, sb->bw, h),
	    (sb->curbutton == 3) ? -1 : 1,
	    AG_COLOR(SCROLLBAR_BTN_COLOR));
}

/* Buttons of horizontal Scrollbar */
static void
ScrollbarHorizButtons(AG_Scrollbar *sb, int x, int w)
{
	int x2;
	
	AG_DrawBox(sb,
	    AG_RECT(0, 0, sb->bw, sb->bw),
	    (sb->curbutton == 1) ? -1 : 1,
	    AG_COLOR(SCROLLBAR_BTN_COLOR));
	AG_DrawArrowLeft(sb, sb->bw/2, sb->bw/2, sb->arrowSz,
	    AG_COLOR(SCROLLBAR_ARR1_COLOR),
	    AG_COLOR(SCROLLBAR_ARR2_COLOR));
	
	x2 = WIDTH(sb)-sb->bw;
	AG_DrawBox(sb,
	    AG_RECT(x2, 0, sb->bw, sb->bw),
	    (sb->curbutton == 2) ? -1 : 1,
	    AG_COLOR(SCROLLBAR_BTN_COLOR));
	AG_DrawArrowRight(sb, (x2 + sb->bw/2), sb->bw/2, sb->arrowSz,
	    AG_COLOR(SCROLLBAR_ARR1_COLOR),
	    AG_COLOR(SCROLLBAR_ARR2_COLOR));

	AG_DrawBox(sb,
	    AG_RECT(sb->bw+x, 0, w, sb->bw),
	    (sb->curbutton == 3) ? -1 : 1,
	    AG_COLOR(SCROLLBAR_BTN_COLOR));
}

/* Horizontal Separator */
static void
SeparatorHoriz(AG_Separator *sep)
{
	AG_DrawLineH(sep, 0, WIDTH(sep), sep->padding,
	    AG_COLOR(SEPARATOR_LINE1_COLOR));
	AG_DrawLineH(sep, 0, WIDTH(sep), sep->padding+1,
	    AG_COLOR(SEPARATOR_LINE2_COLOR));
}

/* Vertical Separator */
static void
SeparatorVert(AG_Separator *sep)
{
	AG_DrawLineV(sep, sep->padding, 0, HEIGHT(sep),
	    AG_COLOR(SEPARATOR_LINE1_COLOR));
	AG_DrawLineV(sep, sep->padding+1, 0, HEIGHT(sep),
	    AG_COLOR(SEPARATOR_LINE2_COLOR));
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
			    AG_COLOR(SOCKET_COLOR));
		} else {
			AG_DrawBoxDithered(sock,
			    AG_RECT(0, 0, WIDTH(sock), HEIGHT(sock)), -1,
			    AG_COLOR(SOCKET_COLOR),
			    AG_COLOR(DISABLED_COLOR));
		}
		break;
	case AG_SOCKET_CIRCLE:
		AG_DrawCircle(sock, WIDTH(sock)/2, HEIGHT(sock)/2,
		    sock->bgData.circle.r, AG_COLOR(SOCKET_COLOR));
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
			    AG_COLOR(SOCKET_HIGHLIGHT_COLOR));
		}
		break;
	case AG_SOCKET_CIRCLE:
		if (highlight) {
			AG_DrawCircle(sock, WIDTH(sock)/2, HEIGHT(sock)/2,
			    sock->bgData.circle.r - sock->lPad,
			    AG_COLOR(SOCKET_HIGHLIGHT_COLOR));
		}
		break;
	}
}

/* Background for table widgets */
static void
TableBackground(void *tbl, int x, int y, int w, int h)
{
	AG_DrawBox(tbl,
	    AG_RECT(x, y, w, h), -1,
	    AG_COLOR(TABLE_COLOR));
}

/* Background for table column headers */
static void
TableColumnHeaderBackground(void *tbl, int idx, AG_Rect r, int isSelected)
{
	AG_DrawBox(tbl, r, isSelected?-1:1, AG_COLOR(TABLE_COLOR));
}

/* Background for table rows */
static void
TableRowBackground(void *wid, AG_Rect r, int isSelected)
{
	if (isSelected) {
		AG_DrawBox(wid, r, 1, AG_COLOR(TABLEVIEW_SEL_COLOR));
	}
}

/* Background for table cells (over TableRowBackground) */
static void
TableCellBackground(void *wid, AG_Rect r, int isSelected)
{
	if (isSelected) {
		Uint8 c[4] = { 0, 0, 250, 64 };
		AG_DrawRectBlended(wid, r, c, AG_ALPHA_SRC);
	}
}

/* Background for textbox widgets */
static void
TextboxBackground(void *tbox, AG_Rect r, int isCombo)
{
	if (AG_WidgetDisabled(tbox)) {
		AG_DrawBoxDithered(tbox, r, -1,
		    AG_COLOR(TEXTBOX_COLOR),
		    AG_COLOR(DISABLED_COLOR));
	} else {
		AG_DrawBox(tbox, r, isCombo?1:-1,
		    AG_COLOR(TEXTBOX_COLOR));
	}
}

/* Background of list widgets */
static void
ListBackground(void *tl, AG_Rect r)
{
	AG_DrawBox(tl, r, -1, AG_COLOR(TLIST_BG_COLOR));
}

/* Background for selected Tlist items */
static void
ListItemBackground(void *tl, AG_Rect r, int isSelected)
{
	if (isSelected) {
		AG_DrawRectFilled(tl, r,
		    AG_COLOR(TLIST_SEL_COLOR));
	}
}

/* Indicate presence of child items in a tree display. */
static void
TreeSubnodeIndicator(void *wid, AG_Rect r, int isExpanded)
{
	Uint8 cBg[4] = { 0, 0, 0, 64 };
	Uint8 cFg[4] = { 255, 255, 255, 100 };

	AG_DrawRectBlended(wid,
	    AG_RECT(r.x-1, r.y, r.w+2, r.h),
	    cBg, AG_ALPHA_SRC);

	if (isExpanded) {
		AG_DrawMinus(wid,
		    AG_RECT(r.x+2, r.y+2, r.w-4, r.h-4),
		    cFg, AG_ALPHA_SRC);
	} else {
		AG_DrawPlus(wid,
		    AG_RECT(r.x+2, r.y+2, r.w-4, r.h-4),
		    cFg, AG_ALPHA_SRC);
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
	WindowBackground,
	WindowBorders,
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
	MenuItemBackground,
	MenuItemSeparator,
	NotebookBackground,
	NotebookTabBackground,
	PaneHorizDivider,
	PaneVertDivider,
	RadioGroupBackground,
	RadioButton,
	ScrollbarBackground,
	ScrollbarVertButtons,
	ScrollbarHorizButtons,
	SeparatorHoriz,
	SeparatorVert,
	SocketBackground,
	SocketOverlay,
	TableBackground,
	TableColumnHeaderBackground,
	TableRowBackground,
	TableCellBackground,
	TextboxBackground,
	ListBackground,
	ListItemBackground,
	TreeSubnodeIndicator
};
