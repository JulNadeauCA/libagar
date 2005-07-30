/*	$Csoft: tool.h,v 1.7 2005/07/24 06:55:57 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_TOOL_H_
#define _AGAR_MAPEDIT_TOOL_H_
#include "begin_code.h"

#define TOOL_STATUS_MAX	8

struct mapview;
struct tool_kbinding;
struct tool_mbinding;

struct tool_ops {
	const char *name;
	const char *desc;
	int icon;

	size_t len;
	int flags;
#define TOOL_HIDDEN	0x01		/* Don't include in toolbars/menus */

	void (*init)(void *);
	void (*destroy)(void *);
	void (*edit_pane)(void *, void *);
	void (*edit)(void *);
	int (*cursor)(void *, SDL_Rect *);
	int (*effect)(void *, struct node *);
	int (*mousemotion)(void *, int x, int y, int xrel, int yrel, int btn);
	int (*mousebuttondown)(void *, int x, int y, int btn);
	int (*mousebuttonup)(void *, int x, int y, int btn);
	int (*keydown)(void *, int ksym, int kmod);
	int (*keyup)(void *, int ksym, int kmod);
};

struct tool {
	const struct tool_ops *ops;

	struct mapview *mv;			/* Associated view */
	void *p;				/* User-supplied pointer */
	char *status[TOOL_STATUS_MAX];		/* Status message stack */
	int nstatus;
	struct window *win;			/* Edition window (if any) */
	struct widget *pane;			/* Edition pane (if any) */
	struct button *trigger;			/* Trigger button (XXX) */

	SLIST_HEAD(,tool_kbinding) kbindings;	/* Keyboard bindings */
	SLIST_HEAD(,tool_mbinding) mbindings;	/* Mouse button bindings */
	TAILQ_ENTRY(tool) tools;
};

struct tool_kbinding {
	SDLMod mod;
	SDLKey key;
	int edit;
	int (*func)(struct tool *, SDLKey k, int s, void *);
	void *arg;
	SLIST_ENTRY(tool_kbinding) kbindings;
};

struct tool_mbinding {
	int button;
	int edit;
	int (*func)(struct tool *, int b, int s, int x, int y, void *);
	void *arg;
	SLIST_ENTRY(tool_mbinding) mbindings;
};

#define TOOL(t) ((struct tool *)(t))

__BEGIN_DECLS
void		 tool_init(struct tool *);
void		 tool_destroy(struct tool *);
struct window	*tool_window(void *, const char *);

void tool_bind_key(void *, SDLMod, SDLKey,
		   int (*)(struct tool *, SDLKey, int, void *), void *);
void tool_bind_mousebutton(void *, int,
			   int (*)(struct tool *, int, int, int, int, void *),
			   void *);
void tool_unbind_key(void *, SDLMod, SDLKey);

void tool_push_status(void *, const char *, ...);
void tool_pop_status(void *);
void tool_update_status(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_TOOL_H_ */
