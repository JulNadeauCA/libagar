/*	Public domain	*/

#ifndef _AGAR_VG_TOOL_H_
#define _AGAR_VG_TOOL_H_
#include <agar/vg/begin.h>

struct vg_tool_keybinding;
struct vg_tool_mousebinding;
struct vg_tool_command;
struct vg_view;

/* VG tool class description */
typedef struct vg_tool_ops {
	const char *name;		/* Tool name */
	const char *desc;		/* Optional description */
	AG_StaticIcon *icon;		/* Optional GUI icon */
	size_t len;			/* Size of instance structure */
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
	int (*keydown)(void *, int ksym, int kmod, Uint32 unicode);
	int (*keyup)(void *, int ksym, int kmod, Uint32 unicode);
} VG_ToolOps;

/* VG tool instance */
typedef struct vg_tool {
	const VG_ToolOps *ops;
	int selected;				/* Tool is in use */
	struct vg_view *vgv;			/* Associated view */
	void *p;				/* User-supplied pointer */
	AG_Window *editWin;			/* Edition window (if any) */
	AG_Widget *editArea;			/* Edition area (if any) */
	VG_Vector vCursor;			/* Last cursor position */
	AG_SLIST_HEAD_(vg_tool_keybinding) kbindings;
	AG_TAILQ_HEAD_(vg_tool_command) cmds;
	AG_TAILQ_ENTRY(vg_tool) tools;
} VG_Tool;

/* General command handler */
typedef struct vg_tool_command {
	char *name;				/* Command string */
	char *descr;				/* Description string */
	AG_Event *fn;				/* Callback routine (bound to VG_View) */
	AG_KeyMod kMod;				/* Bound key modifier */
	AG_KeySym kSym;				/* Bound keysym */
	VG_Tool *tool;				/* Back pointer to tool */
	AG_TAILQ_ENTRY(vg_tool_command) cmds;
} VG_ToolCommand;

#define VGTOOL(t) ((VG_Tool *)(t))
#define VG_CURTOOL(vv) \
    (vv)->curtool != NULL ? (vv)->curtool : \
    (vv)->deftool != NULL ? (vv)->deftool : NULL

__BEGIN_DECLS
void       VG_ToolInit(VG_Tool *);
void       VG_ToolDestroy(VG_Tool *);
AG_Window *VG_ToolWindow(void *, const char *);

VG_ToolCommand *VG_ToolCommandNew(void *, const char *, AG_EventFn);
void            VG_ToolCommandKey(VG_ToolCommand *, AG_KeyMod, AG_KeySym);
void            VG_ToolCommandDescr(VG_ToolCommand *, const char *, ...);
int             VG_ToolCommandExec(void *, const char *, const char *, ...);
__END_DECLS

#include <agar/vg/close.h>
#endif /* _AGAR_VG_TOOL_H_ */
