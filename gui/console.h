/*	Public domain	*/

#ifndef _AGAR_WIDGET_CONSOLE_H_
#define _AGAR_WIDGET_CONSOLE_H_

#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/text.h>

#include <agar/gui/begin.h>

struct ag_console;
struct ag_popup_menu;

/* TODO: timestamps, markup */
typedef struct ag_console_line {
	char *_Nonnull text;    /* Line text */
	AG_Size len;            /* Size in bytes excluding NUL */
	int surface[2];         /* Cached surfaces (0=not selected; 1=selected) */
	AG_Color c;             /* Alternate text color */

	void *_Nullable p;                        /* User pointer */
	struct ag_console *_Nonnull cons;         /* Back pointer to console */
	struct ag_console_line *_Nullable parent; /* Parent line for multi-line groups */
} AG_ConsoleLine;

typedef struct ag_console_file {
	Uint flags;
#define AG_CONSOLE_FILE_BINARY     0x01  /* Display binary in hex dump format */
#define AG_CONSOLE_FILE_LEAVE_OPEN 0x02  /* Don't close FILE* or fd on detach */
	int fd;				 /* File descriptor */
	char *_Nullable label;		 /* Label (e.g., filename or id) */
	void *pFILE;			 /* FILE * pointer */
	AG_Offset offs;			 /* Current file offset */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
	AG_Color *_Nullable color;	 /* Alternate color */
	const AG_NewlineFormat *newline; /* Newline encoding */
	AG_TAILQ_ENTRY(ag_console_file) files;
} AG_ConsoleFile;

typedef struct ag_console {
	struct ag_widget wid;		/* AG_Widget -> AG_Console */

	Uint flags;
#define AG_CONSOLE_HFILL	0x01	/* Fill available width */
#define AG_CONSOLE_VFILL	0x02	/* Fill available height */
#define AG_CONSOLE_NOAUTOSCROLL	0x04	/* Scroll new lines are added */
#define AG_CONSOLE_NOPOPUP	0x08	/* Disable popup menus */
#define AG_CONSOLE_EXPAND	(AG_CONSOLE_HFILL|AG_CONSOLE_VFILL)
#define AG_CONSOLE_SELECTING	0x10	/* Selection in progress */
#define AG_CONSOLE_BEGIN_SELECT 0x20	/* Flag for beginSelectTo */
	Uint32 _pad;
	int lineskip;			/* Space between lines */
	int xOffs;			/* Horizontal display offset (px) */

	AG_ConsoleLine *_Nullable *_Nonnull lines; /* Lines in buffer */
	Uint                               nLines; /* Line count */

	int wMax;			/* Width of widest line seen (px) */

	Uint rOffs;			/* Row display offset */
	Uint rVisible;			/* Visible line count */
	AG_Scrollbar *_Nonnull vBar;	/* Vertical scrollbar */
	AG_Scrollbar *_Nonnull hBar;	/* Horizontal scrollbar */
	AG_Rect r;			/* View area */
	Uint *_Nullable scrollTo;	/* Scrolling request */
	int pos, sel;			/* Position and selection */

	struct ag_popup_menu *_Nullable pm;	/* Active popup menu */
	AG_Timer beginSelectTo;			/* Timer for double-click */
	AG_TAILQ_HEAD_(ag_console_file) files;	/* Files being monitored */
} AG_Console;

#define AGCONSOLE(obj)            ((AG_Console *)(obj))
#define AGCCONSOLE(obj)           ((const AG_Console *)(obj))
#define AG_CONSOLE_SELF()          AGCONSOLE( AG_OBJECT(0,"AG_Widget:AG_Console:*") )
#define AG_CONSOLE_PTR(n)          AGCONSOLE( AG_OBJECT((n),"AG_Widget:AG_Console:*") )
#define AG_CONSOLE_NAMED(n)        AGCONSOLE( AG_OBJECT_NAMED((n),"AG_Widget:AG_Console:*") )
#define AG_CONST_CONSOLE_SELF()   AGCCONSOLE( AG_CONST_OBJECT(0,"AG_Widget:AG_Console:*") )
#define AG_CONST_CONSOLE_PTR(n)   AGCCONSOLE( AG_CONST_OBJECT((n),"AG_Widget:AG_Console:*") )
#define AG_CONST_CONSOLE_NAMED(n) AGCCONSOLE( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Console:*") )

__BEGIN_DECLS
extern AG_WidgetClass agConsoleClass;

AG_Console *_Nonnull AG_ConsoleNew(void *_Nullable, Uint);

AG_ConsoleLine *_Nonnull AG_ConsoleAppendLine(AG_Console *_Nonnull,
                                              const char *_Nullable);
AG_ConsoleLine *_Nonnull AG_ConsoleMsgS(AG_Console *_Nonnull, const char *_Nonnull);
AG_ConsoleLine *_Nonnull AG_ConsoleMsg(AG_Console *_Nonnull, const char *_Nonnull, ...)
                                      FORMAT_ATTRIBUTE(printf,2,3);

void AG_ConsoleBinary(AG_Console *_Nonnull, const void *_Nonnull, AG_Size,
                      const char *_Nullable, const char *_Nullable);

void AG_ConsoleMsgEdit(AG_ConsoleLine *_Nonnull, const char *_Nonnull);
void AG_ConsoleMsgCatS(AG_ConsoleLine *_Nonnull, const char *_Nonnull);
void AG_ConsoleMsgPtr(AG_ConsoleLine *_Nonnull, void *_Nullable);
void AG_ConsoleMsgColor(AG_ConsoleLine *_Nonnull, const AG_Color *_Nonnull);
void AG_ConsoleClear(AG_Console *_Nonnull);

char *_Nullable AG_ConsoleExportText(const AG_Console *_Nonnull, enum ag_newline_type);
char *_Nullable AG_ConsoleExportBuffer(const AG_Console *_Nonnull, enum ag_newline_type);

AG_ConsoleFile *AG_ConsoleOpenFile(AG_Console *_Nonnull, const char *_Nullable,
                                   const char *_Nullable, enum ag_newline_type,
				   Uint);
AG_ConsoleFile *AG_ConsoleOpenFD(AG_Console *_Nonnull, const char *_Nullable,
                                     int, enum ag_newline_type, Uint);
AG_ConsoleFile *AG_ConsoleOpenStream(AG_Console *_Nonnull, const char *_Nullable,
                                     void *_Nullable, enum ag_newline_type, Uint);
void            AG_ConsoleClose(AG_Console *_Nonnull, AG_ConsoleFile *_Nonnull);

#ifdef AG_LEGACY
# define AG_ConsoleSetFont(cons,font) AG_SetFont((cons),(font))
void AG_ConsoleSetPadding(AG_Console *_Nonnull, int)
                         DEPRECATED_ATTRIBUTE;
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_CONSOLE_H_ */
