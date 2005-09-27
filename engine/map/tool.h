/*	$Csoft: tool.h,v 1.9 2005/08/01 04:09:13 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_TOOL_H_
#define _AGAR_MAPEDIT_TOOL_H_
#include "begin_code.h"

#define AG_MAPTOOL_STATUS_MAX	8

struct ag_mapview;
struct ag_maptool_keybinding;
struct ag_maptool_mousebinding;

typedef struct ag_maptool_ops {
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
	int (*effect)(void *, AG_Node *);
	int (*mousemotion)(void *, int x, int y, int xrel, int yrel, int btn);
	int (*mousebuttondown)(void *, int x, int y, int btn);
	int (*mousebuttonup)(void *, int x, int y, int btn);
	int (*keydown)(void *, int ksym, int kmod);
	int (*keyup)(void *, int ksym, int kmod);
} AG_MaptoolOps;

typedef struct ag_maptool {
	const AG_MaptoolOps *ops;
	struct ag_mapview *mv;			/* Associated view */
	void *p;				/* User-supplied pointer */
	char *status[AG_MAPTOOL_STATUS_MAX];	/* Status message stack */
	int nstatus;
	AG_Window *win;		/* Edition window (if any) */
	AG_Widget *pane;		/* Edition pane (if any) */
	AG_Button *trigger;		/* Trigger button (XXX) */

	SLIST_HEAD(,ag_maptool_keybinding) kbindings;
	SLIST_HEAD(,ag_maptool_mousebinding) mbindings;
	TAILQ_ENTRY(ag_maptool) tools;
} AG_Maptool;

typedef struct ag_maptool_keybinding {
	SDLMod mod;
	SDLKey key;
	int edit;
	int (*func)(AG_Maptool *, SDLKey k, int s, void *);
	void *arg;
	SLIST_ENTRY(ag_maptool_keybinding) kbindings;
} AG_MaptoolKeyBinding;

typedef struct ag_maptool_mousebinding {
	int button;
	int edit;
	int (*func)(AG_Maptool *, int b, int s, int x, int y, void *);
	void *arg;
	SLIST_ENTRY(ag_maptool_mousebinding) mbindings;
} AG_MaptoolMouseBinding;

#define TOOL(t) ((AG_Maptool *)(t))

__BEGIN_DECLS
void		 AG_MaptoolInit(AG_Maptool *);
void		 AG_MaptoolDestroy(AG_Maptool *);
AG_Window	*AG_MaptoolWindow(void *, const char *);

void AG_MaptoolBindKey(void *, SDLMod, SDLKey,
		   int (*)(AG_Maptool *, SDLKey, int, void *), void *);
void AG_MaptoolBindMouseButton(void *, int,
			   int (*)(AG_Maptool *, int, int, int, int, void *),
			   void *);
void AG_MaptoolUnbindKey(void *, SDLMod, SDLKey);

void AG_MaptoolPushStatus(void *, const char *, ...);
void AG_MaptoolSetStatus(void *, const char *, ...);
void AG_MaptoolPopStatus(void *);
void AG_MaptoolUpdateStatus(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_TOOL_H_ */
