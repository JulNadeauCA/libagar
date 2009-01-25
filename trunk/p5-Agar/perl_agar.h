/*	Public domain	*/

typedef unsigned char Uchar;
typedef unsigned int Uint;
typedef unsigned long Ulong;

typedef struct ap_flag_names {
	const char *name;
	Uint bitmask;
} AP_FlagNames;

/* Translate a hashref of options to a set of bit flags. */
static __inline__ void
AP_MapHashToFlags(void *pHV, const AP_FlagNames *map, Uint *pFlags)
{
	SV **val;
	int i;

	for (i = 0; map[i].name != NULL; i++) {
		val = hv_fetch((HV *)pHV, map[i].name, strlen(map[i].name), 0);
		if (val)
			*pFlags |= map[i].bitmask;
	}
}

/* Functions to allow Widget subclasses to access flags in their superclass. */
static __inline__ int
AP_SetNamedFlag(const char *str, const AP_FlagNames *map, Uint *flags)
{
	int i;
	for (i = 0; map[i].name != NULL; i++) {
		if (strEQ(str, map[i].name)) {
			*flags |= map[i].bitmask;
			return 0;
		}
	}
	return -1;
}
static __inline__ int
AP_UnsetNamedFlag(const char *str, const AP_FlagNames *map, Uint *flags)
{
	int i;
	for (i = 0; map[i].name != NULL; i++) {
		if (strEQ(str, map[i].name)) {
			*flags &= ~(map[i].bitmask);
			return 0;
		}
	}
	return -1;
}
static __inline__ int
AP_GetNamedFlag(const char *str, const AP_FlagNames *map, Uint flags, Uint *flag)
{
	int i;
	for (i = 0; map[i].name != NULL; i++) {
		if (strEQ(str, map[i].name)) {
			*flag = flags & map[i].bitmask;
			return 0;
		}
	}
	return -1;
}
static __inline__ int
AP_SetNamedFlagSigned(const char *str, const AP_FlagNames *map, int *flags)
{
	int i;
	for (i = 0; map[i].name != NULL; i++) {
		if (strEQ(str, map[i].name)) {
			*flags |= map[i].bitmask;
			return 0;
		}
	}
	return -1;
}
static __inline__ int
AP_UnsetNamedFlagSigned(const char *str, const AP_FlagNames *map, int *flags)
{
	int i;
	for (i = 0; map[i].name != NULL; i++) {
		if (strEQ(str, map[i].name)) {
			*flags &= ~(map[i].bitmask);
			return 0;
		}
	}
	return -1;
}

/* defined in Event.xs */
extern void AP_StoreEventPV(AG_Event *event, SV *pv);
extern void AP_EventHandler(AG_Event *event);
extern void AP_EventHandlerDecRef(AG_Event *event);
extern SV * AP_RetrieveEventPV(AG_Event *event);
extern void AP_DecRefEventPV(AG_Event *event);

/* Flags used by all widget types. */
static const AP_FlagNames AP_WidgetFlagNames[] = {
	{ "hFill",		AG_WIDGET_HFILL },
	{ "vFill",		AG_WIDGET_VFILL },
	{ "hide",		AG_WIDGET_HIDE },
	{ "disabled",		AG_WIDGET_DISABLED },
	{ "focusable",		AG_WIDGET_FOCUSABLE },
	{ "unfocusedMotion",	AG_WIDGET_UNFOCUSED_MOTION },
	{ "unfocusedButtonUp",	AG_WIDGET_UNFOCUSED_BUTTONUP },
	{ "unfocusedButtonDown",AG_WIDGET_UNFOCUSED_BUTTONDOWN },
	{ "catchTab",		AG_WIDGET_CATCH_TAB },
	{ "noSpacing",		AG_WIDGET_NOSPACING }
};

typedef AG_Object * Agar__Object;
typedef AG_Widget * Agar__Widget;
typedef AG_Event * Agar__Event;
typedef AG_PixelFormat * Agar__PixelFormat;
typedef AG_Surface * Agar__Surface;
typedef AG_Window * Agar__Window;
typedef AG_Config * Agar__Config;
typedef AG_Font * Agar__Font;
typedef AG_Box * Agar__Box;
typedef AG_Button * Agar__Button;
typedef AG_Checkbox * Agar__Checkbox;
typedef AG_Combo * Agar__Combo;
typedef AG_Console * Agar__Console;
typedef AG_Editable * Agar__Editable;
typedef AG_FileDlg * Agar__FileDlg;
typedef AG_Fixed * Agar__Fixed;
typedef AG_GLView * Agar__GLView;
typedef AG_Graph * Agar__Graph;
typedef AG_FixedPlotter * Agar__FixedPlotter;
typedef AG_HSVPal * Agar__HSVPal;
typedef AG_Icon * Agar__Icon;
typedef AG_Label * Agar__Label;
typedef AG_MPane * Agar__MPane;
typedef AG_Menu * Agar__Menu;
typedef AG_MenuItem * Agar__MenuItem;
typedef AG_Notebook * Agar__Notebook;
typedef AG_NotebookTab * Agar__NotebookTab;
typedef AG_Numerical * Agar__Numerical;
typedef AG_Pane * Agar__Pane;
typedef AG_Pixmap * Agar__Pixmap;
typedef AG_PopupMenu * Agar__PopupMenu;
typedef AG_ProgressBar * Agar__ProgressBar;
typedef AG_Radio * Agar__Radio;
typedef AG_Scrollbar * Agar__Scrollbar;
typedef AG_Scrollview * Agar__Scrollview;
typedef AG_Separator * Agar__Separator;
typedef AG_Slider * Agar__Slider;
typedef AG_Socket * Agar__Socket;
typedef AG_Statusbar * Agar__Statusbar;
typedef AG_Table * Agar__Table;
typedef AG_Textbox * Agar__Textbox;
typedef AG_Tlist * Agar__Tlist;
typedef AG_TlistItem * Agar__TlistItem;
typedef AG_Toolbar * Agar__Toolbar;
typedef AG_UCombo * Agar__UCombo;
typedef SDL_Surface * SDL__Surface;
typedef SDL_Event * SDL__Event;
