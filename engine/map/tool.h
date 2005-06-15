/*	$Csoft: tool.h,v 1.1 2005/04/14 06:19:41 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_TOOL_H_
#define _AGAR_MAPEDIT_TOOL_H_
#include "begin_code.h"

#define TOOL_STATUS_MAX	8

struct mapview;
struct tool_kbinding;
struct tool_mbinding;

struct tool {
	const char *name;
	const char *desc;
	int icon;
	int cursor_index;			/* Static cursor (or -1) */

	void (*init)(struct tool *);
	void (*destroy)(struct tool *);
	int (*load)(struct tool *, struct netbuf *);
	int (*save)(struct tool *, struct netbuf *);
	int (*cursor)(struct tool *, SDL_Rect *);
	int (*effect)(struct tool *, struct node *);
	int (*mousemotion)(struct tool *, int x, int y, int xrel, int yrel,
	                   int xo, int yo, int xorel, int yorel, int b);
	int (*mousebuttondown)(struct tool *, int x, int y, int xoff,
	                       int yoff, int b);
	int (*mousebuttonup)(struct tool *, int x, int y, int xoff, int yoff,
	                     int b);
	void (*keydown)(struct tool *, int ksym, int kmod);
	void (*keyup)(struct tool *, int ksym, int kmod);
	
	char *status[TOOL_STATUS_MAX];		/* Status message stack */
	int nstatus;
	struct mapview *mv;			/* Associated mapview */
	void *p;				/* User-supplied pointer */
	SDL_Surface *cursor_su;			/* Static cursor surface */

	SLIST_HEAD(,tool_kbinding) kbindings;	/* Keyboard bindings */
	SLIST_HEAD(,tool_mbinding) mbindings;	/* Mouse button bindings */

	struct window *win;
	struct button *trigger;

	TAILQ_ENTRY(tool) tools;
};

struct tool_kbinding {
	SDLMod mod;
	SDLKey key;
	int edit;				/* Require edition mode */
	void (*func)(struct tool *, int);
	SLIST_ENTRY(tool_kbinding) kbindings;
};

struct tool_mbinding {
	int button;
	int edit;				/* Require edition mode */
	int override;				/* Override defaults */
	void (*func)(struct tool *, int);
	SLIST_ENTRY(tool_mbinding) mbindings;
};

__BEGIN_DECLS
void		 tool_init(struct tool *, struct mapview *);
void		 tool_destroy(struct tool *);
struct window	*tool_window(void *, const char *);
void		 tool_bind_key(void *, SDLMod, SDLKey,
		               void (*)(struct tool *, int), int);
void		 tool_bind_mousebutton(void *, int, int,
		                       void (*)(struct tool *, int), int);
void		 tool_unbind_key(void *, SDLMod, SDLKey);
void		 tool_push_status(struct tool *, const char *, ...);
void		 tool_pop_status(struct tool *);
void		 tool_update_status(struct tool *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_TOOL_H_ */
