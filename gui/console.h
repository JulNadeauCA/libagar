/*	Public domain	*/

#ifndef _AGAR_WIDGET_CONSOLE_H_
#define _AGAR_WIDGET_CONSOLE_H_

#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

struct ag_console;
struct ag_popup_menu;

typedef struct ag_console_line {
	char *_Nonnull text;			/* Line text */
	AG_Size len;				/* Length not including NUL */
	int surface[2];				/* Cached surface handle (or -1) */
	int icon;				/* Icon to display */
	AG_Color cAlt;				/* Alternate text color */
	void *_Nullable p;			/* User pointer */
	struct ag_console *_Nonnull cons;	/* Back pointer to console */
} AG_ConsoleLine;

typedef struct ag_console {
	struct ag_widget wid;
	Uint flags;
#define AG_CONSOLE_HFILL	0x01	/* Fill available width */
#define AG_CONSOLE_VFILL	0x02	/* Fill available height */
#define AG_CONSOLE_NOAUTOSCROLL	0x04	/* Scroll new lines are added */
#define AG_CONSOLE_NOPOPUP	0x08	/* Disable popup menus */
#define AG_CONSOLE_EXPAND	(AG_CONSOLE_HFILL|AG_CONSOLE_VFILL)
#define AG_CONSOLE_SELECTING	0x10	/* Selection in progress */
	int padding;			/* Padding in pixels */
	int lineskip;			/* Space between lines */

	AG_ConsoleLine *_Nullable *_Nonnull lines; /* Lines in buffer */
	Uint            nLines;			   /* Line count */

	Uint rOffs;			/* Row display offset */
	AG_Scrollbar *_Nonnull vBar;	/* Vertical scrollbar */
	AG_Rect r;			/* View area */
	Uint rVisible;			/* Visible line count */
	Uint *_Nullable scrollTo;	/* Scrolling request */
	int pos, sel;			/* Position and selection */
	struct ag_popup_menu *_Nullable pm; /* Active popup menu */
} AG_Console;

__BEGIN_DECLS
extern AG_WidgetClass agConsoleClass;

AG_Console *_Nonnull AG_ConsoleNew(void *_Nullable, Uint);

AG_ConsoleLine *_Nullable AG_ConsoleAppendLine(AG_Console *_Nonnull,
                                               const char *_Nullable);
AG_ConsoleLine *_Nullable AG_ConsoleMsgS(AG_Console *_Nullable,
                                         const char *_Nonnull);
AG_ConsoleLine *_Nullable AG_ConsoleMsg(AG_Console *_Nullable,
                                        const char *_Nonnull, ...)
                                       FORMAT_ATTRIBUTE(printf,2,3);

void    AG_ConsoleSetPadding(AG_Console *_Nonnull, int);
void    AG_ConsoleMsgEdit(AG_ConsoleLine *_Nonnull, const char *_Nonnull);
void    AG_ConsoleMsgPtr(AG_ConsoleLine *_Nonnull, void *_Nullable);
void    AG_ConsoleMsgIcon(AG_ConsoleLine *_Nonnull, int);
void    AG_ConsoleMsgColor(AG_ConsoleLine *_Nonnull, AG_Color);
void    AG_ConsoleClear(AG_Console *_Nonnull);

char *_Nullable AG_ConsoleExportText(AG_Console *_Nonnull, int);

#ifdef AG_LEGACY
# define AG_ConsoleSetFont(cons,font) AG_SetFont((cons),(font))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_CONSOLE_H_ */
