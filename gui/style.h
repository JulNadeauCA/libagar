/*	Public domain	*/

#ifndef _AGAR_GUI_STYLE_H_
#define _AGAR_GUI_STYLE_H_
#include <agar/gui/begin.h>

struct ag_window;
struct ag_radio;
struct ag_scrollbar;
struct ag_separator;
struct ag_socket;

typedef struct ag_style {
	const char *name;
	struct {
		int maj, min;
	} version;
	void (*init)(struct ag_style *);
	void (*destroy)(struct ag_style *);
	void (*Window)(struct ag_window *);
	void (*TitlebarBackground)(void *, int isPressed, int isFocused);
	void (*ButtonBackground)(void *, int isPressed);
	void (*ButtonTextOffset)(void *, int isPressed, int *x, int *y);
	void (*BoxFrame)(void *, AG_Rect r, int depth);
	void (*CheckboxButton)(void *, int state, int size);
	void (*ConsoleBackground)(void *, AG_Color bg);
	void (*FixedPlotterBackground)(void *, int showAxis, Uint32 yOffs);
	void (*MenuRootBackground)(void *);
	void (*MenuRootSelectedItemBackground)(void *, AG_Rect r);
	void (*MenuBackground)(void *, AG_Rect r);
	void (*MenuItemBackground)(void *, AG_Rect r, int xIcon, void *iconObj,
	                           int icon, int isSelected, int boolState);
	void (*MenuItemSeparator)(void *, int x1, int x2, int y, int h);
	void (*NotebookBackground)(void *, AG_Rect r);
	void (*NotebookTabBackground)(void *, AG_Rect r, int idx,
	                              int isSelected);
	void (*PaneHorizDivider)(void *, int x, int y, int w, int isMoving);
	void (*PaneVertDivider)(void *, int x, int y, int w, int isMoving);
	void (*RadioGroupBackground)(void *, AG_Rect r);
	void (*RadioButton)(struct ag_radio *, int x, int y, int selected,
	                    int over);
	void (*ProgressBarBackground)(void *);
	void (*ScrollbarVert)(struct ag_scrollbar *, int y, int h);
	void (*ScrollbarHoriz)(struct ag_scrollbar *, int x, int w);
	void (*SliderBackgroundHoriz)(void *);
	void (*SliderBackgroundVert)(void *);
	void (*SliderControlHoriz)(void *, int, int);
	void (*SliderControlVert)(void *, int, int);
	void (*SeparatorHoriz)(struct ag_separator *);
	void (*SeparatorVert)(struct ag_separator *);
	void (*SocketBackground)(struct ag_socket *);
	void (*SocketOverlay)(struct ag_socket *, int highlight);
	void (*TableBackground)(void *, AG_Rect r);
	void (*TableColumnHeaderBackground)(void *, int idx, AG_Rect r,
	                                    int isSelected);
	void (*TableSelectedColumnBackground)(void *, int idx, AG_Rect r);
	void (*TableRowBackground)(void *, AG_Rect r, int isSelected);
	void (*TextboxBackground)(void *, AG_Rect r, int isCombo);
	void (*ListBackground)(void *, AG_Rect r);
	void (*ListItemBackground)(void *, AG_Rect r, int isSelected);
	void (*TreeSubnodeIndicator)(void *, AG_Rect r, int isExpanded);
} AG_Style;

__BEGIN_DECLS
extern AG_Style agStyleDefault;
void AG_SetStyle(void *, AG_Style *);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_STYLE_H_ */
