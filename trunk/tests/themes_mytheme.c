/*	Public domain	*/
/*
 * Custom theme example. The default theme is implemented in
 * gui/style_default.c. You will probably want to use that file
 * as a template.
 *
 * See AG_Style(3) for a list of all style functions.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/dev.h>

#include "mytheme.h"

#define WIDTH(wd)  AGWIDGET(wd)->w
#define HEIGHT(wd) AGWIDGET(wd)->h

static void
TitlebarBackground(void *tbar, int isPressed, int windowIsFocused)
{
	AG_DrawBoxRoundedTop(tbar,
	    AG_RECT(0, 0, WIDTH(tbar), HEIGHT(tbar)),
	    isPressed ? -1 : 1,
	    8,
	    windowIsFocused ? agColors[TITLEBAR_FOCUSED_COLOR] :
	                      agColors[TITLEBAR_UNFOCUSED_COLOR]);
}

static void
ButtonBackground(void *btn, int isPressed)
{
	AG_DrawBoxRounded(btn,
	    AG_RECT(0, 0, WIDTH(btn), HEIGHT(btn)),
	    isPressed ? -1 : 1,
	    6,
	    agColors[BUTTON_COLOR]);
}

static void
CheckboxButton(void *cbox, int state, int size)
{
	AG_DrawBoxRounded(cbox,
	    AG_RECT(0, 0, size, size),
	    state ? -1 : 1,
	    6,
	    agColors[CHECKBOX_COLOR]);
}

static void
TextboxBackground(void *tbox, AG_Rect r, int isCombo)
{
	AG_DrawBoxRounded(tbox, r, isCombo?1:-1, 6,
	    agColors[TEXTBOX_COLOR]);
}

/*
 * Note: The layout of AG_Style may change, so we override individual members
 * instead of using a static initializer.
 */
AG_Style myRoundedStyle;

void
InitMyRoundedStyle(AG_Style *s)
{
	*s = agStyleDefault;			/* Inherit from default */
	s->name = "rounded";
	s->version.maj = 1;
	s->version.min = 0;
	s->init = NULL;
	s->destroy = NULL;
	s->TitlebarBackground = TitlebarBackground;
	s->ButtonBackground = ButtonBackground;
	s->CheckboxButton = CheckboxButton;
	s->TextboxBackground = TextboxBackground;
}
