/*	Public domain	*/

#ifndef _AGAR_WIDGET_CONSOLE_H_
#define _AGAR_WIDGET_CONSOLE_H_

#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

#define AG_CONSOLE_LINE_MAX	1024

struct ag_console;
struct ag_popup_menu;

typedef struct ag_console_line {
	char *text;			/* Line text */
	size_t len;			/* Length not including NUL */
	int surface[2];			/* Cached surface handle (or -1) */
	int icon;			/* Icon to display */
	AG_Color cAlt;			/* Alternate text color */
	void *p;			/* User pointer */
	struct ag_console *cons;	/* Back pointer to Console */
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
	AG_ConsoleLine **lines;		/* Lines in buffer */
	Uint nLines;			/* Line count */
	Uint rOffs;			/* Row display offset */
	AG_Scrollbar *vBar;		/* Scrollbar */
	AG_Rect r;			/* View area */
	Uint rVisible;			/* Visible line count */
	Uint *scrollTo;			/* Scrolling request */
	int pos, sel;			/* Position and selection */
	struct ag_popup_menu *pm;
} AG_Console;

__BEGIN_DECLS
extern AG_WidgetClass agConsoleClass;

AG_Console     *AG_ConsoleNew(void *, Uint);
void		AG_ConsoleSetPadding(AG_Console *, int);
void            AG_ConsoleSetFont(AG_Console *, AG_Font *);
AG_ConsoleLine *AG_ConsoleAppendLine(AG_Console *, const char *);
AG_ConsoleLine *AG_ConsoleMsg(AG_Console *, const char *, ...)
                              FORMAT_ATTRIBUTE(printf, 2, 3)
                              NONNULL_ATTRIBUTE(2);
AG_ConsoleLine *AG_ConsoleMsgS(AG_Console *, const char *);
void            AG_ConsoleMsgEdit(AG_ConsoleLine *, const char *);
void		AG_ConsoleMsgPtr(AG_ConsoleLine *, void *);
void		AG_ConsoleMsgIcon(AG_ConsoleLine *, int);
void            AG_ConsoleMsgColor(AG_ConsoleLine *, const AG_Color *);
void		AG_ConsoleClear(AG_Console *);
char           *AG_ConsoleExportText(AG_Console *, int);

#ifdef AG_LEGACY
# define AG_ConsoleSetFont(cons,font) AG_SetFont((cons),(font))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_CONSOLE_H_ */
