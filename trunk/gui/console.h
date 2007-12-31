/*	Public domain	*/

#ifndef _AGAR_WIDGET_CONSOLE_H_
#define _AGAR_WIDGET_CONSOLE_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/scrollbar.h>
#include <gui/text.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/text.h>
#endif

#include "begin_code.h"

#define AG_CONSOLE_LINE_MAX	1024

struct ag_console;

typedef struct ag_console_line {
	char *text;			/* Line text */
	size_t len;			/* Length not including NUL */
	int surface;			/* Cached surface handle (or -1) */
	int selected;			/* Row is selected */
	int icon;			/* Icon to display */
	AG_Font *font;			/* Font */
	Uint32 cFg;			/* Foreground color (display fmt) */
	Uint32 cBg;			/* Background color (display fmt) */
	void *p;			/* User pointer */
	struct ag_console *cons;	/* Back pointer to Console */
} AG_ConsoleLine;

typedef struct ag_console {
	struct ag_widget wid;
	Uint flags;
#define AG_CONSOLE_HFILL	0x01	/* Fill available width */
#define AG_CONSOLE_VFILL	0x02	/* Fill available height */
#define AG_CONSOLE_EXPAND	(AG_CONSOLE_HFILL|AG_CONSOLE_VFILL)
	int padding;			/* Padding in pixels */
	int lineskip;			/* Space between lines */
	AG_ConsoleLine *lines;		/* Lines in buffer */
	Uint nLines;			/* Line count */
	Uint rOffs;			/* Row display offset */
	Uint32 cBg;			/* Background color */
	AG_Scrollbar *vBar;		/* Scrollbar */
} AG_Console;

__BEGIN_DECLS
extern AG_WidgetClass agConsoleClass;

AG_Console     *AG_ConsoleNew(void *, Uint);
void		AG_ConsoleSetPadding(AG_Console *, int);
AG_ConsoleLine *AG_ConsoleAppendLine(AG_Console *, const char *);
AG_ConsoleLine *AG_ConsoleMsg(AG_Console *, const char *, ...)
			         FORMAT_ATTRIBUTE(printf, 2, 3)
			         NONNULL_ATTRIBUTE(2);
void		AG_ConsoleMsgPtr(AG_ConsoleLine *, void *);
void		AG_ConsoleMsgIcon(AG_ConsoleLine *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CONSOLE_H_ */
