/*	$Csoft: tool.h,v 1.3 2004/04/10 04:55:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_TOOL_H_
#define _AGAR_MAPEDIT_TOOL_H_
#include "begin_code.h"

#define TOOL_STATUS_MAX	8

struct mapview;
struct tool_kbinding;

struct tool {
	const char *name;
	const char *desc;
	int icon;
	int cursor_index;			/* Static cursor (or -1) */

	void (*init)(struct tool *t);
	void (*destroy)(struct tool *t);
	int  (*load)(struct tool *t, struct netbuf *b);
	int  (*save)(struct tool *t, struct netbuf *b);
	int  (*cursor)(struct tool *t, SDL_Rect *r);
	void (*effect)(struct tool *t, struct node *n);
	void (*mousemotion)(struct tool *t, int x, int y, int xrel, int yrel,
	                    int xo, int yo, int xorel, int yorel, int b);
	void (*mousebuttondown)(struct tool *t, int x, int y, int xoff,
	                        int yoff, int b);
	void (*mousebuttonup)(struct tool *t, int x, int y, int xoff, int yoff,
	                      int b);
	void (*keydown)(struct tool *t, int ksym, int kmod);
	void (*keyup)(struct tool *t, int ksym, int kmod);
	
	char *status[TOOL_STATUS_MAX];		/* Status message stack */
	int nstatus;
	struct mapview *mv;			/* Associated mapview */
	void *p;				/* User-supplied pointer */
	SDL_Surface *cursor_su;			/* Static cursor surface */
	SLIST_HEAD(,tool_kbinding) kbindings;	/* Keyboard bindings */

	struct window *win;
	struct button *trigger;
	TAILQ_ENTRY(tool) tools;
};

struct tool_kbinding {
	const char *name;
	SDLMod mod;
	SDLKey key;
	int edit;				/* Require edition mode */
	void (*func)(struct tool *, int);
	SLIST_ENTRY(tool_kbinding) kbindings;
};

__BEGIN_DECLS
void		 tool_init(struct tool *, struct mapview *);
void		 tool_destroy(struct tool *);
struct window	*tool_window(void *, const char *);
void		 tool_bind_key(void *, SDLMod, SDLKey,
		               void (*)(struct tool *, int), int);
void		 tool_unbind_key(void *, SDLMod, SDLKey);
void		 tool_push_status(struct tool *, const char *, ...);
void		 tool_pop_status(struct tool *);
void		 tool_update_status(struct tool *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_TOOL_H_ */
