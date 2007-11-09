/*	$Csoft: tool.h,v 1.9 2005/08/01 04:09:13 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_TOOL_H_
#define _AGAR_VG_TOOL_H_
#include "begin_code.h"

struct vg_tool_keybinding;
struct vg_tool_mousebinding;
struct vg_view;

typedef struct vg_tool_ops {
	const char *name, *desc;
	AG_StaticIcon *icon;
	size_t len;
	Uint flags;
#define VG_MOUSEMOTION_NOSNAP	0x01	/* Ignore snapping in mousemotion */
#define VG_BUTTONUP_NOSNAP	0x02	/* Ignore snapping in buttonup */
#define VG_BUTTONDOWN_NOSNAP	0x04	/* Ignore snapping in buttondown */
#define VG_BUTTON_NOSNAP	(VG_BUTTONUP_NOSNAP|VG_BUTTONDOWN_NOSNAP)
#define VG_NOSNAP		(VG_BUTTON_NOSNAP|VG_MOUSEMOTION_NOSNAP)

	void (*init)(void *);
	void (*destroy)(void *);
	void (*edit)(void *, void *);
	int (*mousemotion)(void *, float x, float y, float xrel, float yrel,
	                   int btn);
	int (*mousebuttondown)(void *, float x, float y, int btn);
	int (*mousebuttonup)(void *, float x, float y, int btn);
	int (*keydown)(void *, int ksym, int kmod);
	int (*keyup)(void *, int ksym, int kmod);
} VG_ToolOps;

typedef struct vg_tool {
	const VG_ToolOps *ops;
	struct vg_view *vgv;			/* Associated view */
	void *p;				/* User-supplied pointer */
	AG_Window *win;				/* Edition window (if any) */
	AG_Widget *pane;			/* Edition pane (if any) */
	AG_Button *trigger;			/* Trigger button (XXX) */
	SLIST_HEAD(,vg_tool_keybinding) kbindings;
	SLIST_HEAD(,vg_tool_mousebinding) mbindings;
	TAILQ_ENTRY(vg_tool) tools;
} VG_Tool;

typedef struct vg_tool_keybinding {
	SDLMod mod;
	SDLKey key;
	int edit;
	int (*func)(VG_Tool *, SDLKey k, int s, void *);
	void *arg;
	SLIST_ENTRY(vg_tool_keybinding) kbindings;
} VG_ToolKeyBinding;

typedef struct vg_tool_mousebinding {
	int button;
	int edit;
	int (*func)(VG_Tool *, int b, int s, float x, float y, void *);
	void *arg;
	SLIST_ENTRY(vg_tool_mousebinding) mbindings;
} VG_ToolMouseBinding;

#define VGTOOL(t) ((VG_Tool *)(t))
#define VG_CURTOOL(vv) \
    (vv)->curtool != NULL ? (vv)->curtool : \
    (vv)->deftool != NULL ? (vv)->deftool : NULL

__BEGIN_DECLS
void		 VG_ToolInit(VG_Tool *);
void		 VG_ToolDestroy(VG_Tool *);
AG_Window	*VG_ToolWindow(void *, const char *);

void VG_ToolBindKey(void *, SDLMod, SDLKey,
		    int (*)(VG_Tool *, SDLKey, int, void *), void *);
void VG_ToolBindMouseButton(void *, int,
			    int (*)(VG_Tool *, int, int, float, float, void *),
			    void *);
void VG_ToolUnbindKey(void *, SDLMod, SDLKey);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_TOOL_H_ */
