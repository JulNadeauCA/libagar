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
#define VG_NOEDITCLEAR		0x08	/* Don't clear edit areas on select */

	void (*init)(void *);
	void (*destroy)(void *);
	void *(*edit)(void *, struct vg_view *);
	void (*predraw)(void *, struct vg_view *);
	void (*postdraw)(void *, struct vg_view *);
	void (*selected)(void *, struct vg_view *);
	void (*deselected)(void *, struct vg_view *);

	int (*mousemotion)(void *, VG_Vector vPos, VG_Vector vRel, int buttons);
	int (*mousebuttondown)(void *, VG_Vector vPos, int button);
	int (*mousebuttonup)(void *, VG_Vector vPos, int button);
	int (*keydown)(void *, int ksym, int kmod, int unicode);
	int (*keyup)(void *, int ksym, int kmod, int unicode);
} VG_ToolOps;

typedef struct vg_tool {
	const VG_ToolOps *ops;
	int selected;				/* Tool is in use */
	struct vg_view *vgv;			/* Associated view */
	void *p;				/* User-supplied pointer */
	AG_Window *editWin;			/* Edition window (if any) */
	AG_Widget *editArea;			/* Edition area (if any) */
	VG_Vector vCursor;			/* Last cursor position */
	AG_SLIST_HEAD(,vg_tool_keybinding) kbindings;
	AG_SLIST_HEAD(,vg_tool_mousebinding) mbindings;
	AG_TAILQ_ENTRY(vg_tool) tools;
} VG_Tool;

typedef struct vg_tool_keybinding {
	SDLMod mod;
	SDLKey key;
	int edit;
	int (*func)(VG_Tool *, SDLKey k, int s, void *);
	void *arg;
	AG_SLIST_ENTRY(vg_tool_keybinding) kbindings;
} VG_ToolKeyBinding;

typedef struct vg_tool_mousebinding {
	int button;
	int edit;
	int (*func)(VG_Tool *, int b, int s, float x, float y, void *);
	void *arg;
	AG_SLIST_ENTRY(vg_tool_mousebinding) mbindings;
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
