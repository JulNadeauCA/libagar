/*	Public domain	*/

#ifndef _AGAR_VG_TOOL_H_
#define _AGAR_VG_TOOL_H_

#include <agar/gui/iconmgr.h>

#include <agar/vg/begin.h>

struct vg_tool_keybinding;
struct vg_tool_mousebinding;
struct vg_tool_command;
struct vg_view;

/* VG tool class description */
typedef struct vg_tool_ops {
	const char *_Nonnull  name;	/* Display text */
	const char *_Nonnull  desc;	/* Display description */
	AG_StaticIcon *_Nullable icon;	/* Optional GUI icon */
	AG_Size len;			/* Size of instance structure */
#if AG_MODEL == AG_LARGE
	Uint64 flags;
#else
	Uint flags;
#endif
#define VG_MOUSEMOTION_NOSNAP	0x01	/* Ignore snapping in mousemotion */
#define VG_BUTTONUP_NOSNAP	0x02	/* Ignore snapping in buttonup */
#define VG_BUTTONDOWN_NOSNAP	0x04	/* Ignore snapping in buttondown */
#define VG_BUTTON_NOSNAP	(VG_BUTTONUP_NOSNAP|VG_BUTTONDOWN_NOSNAP)
#define VG_NOSNAP		(VG_BUTTON_NOSNAP|VG_MOUSEMOTION_NOSNAP)
#define VG_NOEDITCLEAR		0x08	/* Don't clear edit areas on select */

	void (*_Nullable init)(void *_Nonnull);
	void (*_Nullable destroy)(void *_Nonnull);

	void *_Nullable (*_Nullable edit)(void *_Nonnull, struct vg_view *_Nonnull);

	void (*_Nullable predraw)(void *_Nonnull, struct vg_view *_Nonnull);
	void (*_Nullable postdraw)(void *_Nonnull, struct vg_view *_Nonnull);
	void (*_Nullable selected)(void *_Nonnull, struct vg_view *_Nonnull);
	void (*_Nullable deselected)(void *_Nonnull, struct vg_view *_Nonnull);

	int (*_Nullable mousemotion)(void *_Nonnull, VG_Vector, VG_Vector, int);
	int (*_Nullable mousebuttondown)(void *_Nonnull, VG_Vector, int);
	int (*_Nullable mousebuttonup)(void *_Nonnull, VG_Vector, int);
	int (*_Nullable keydown)(void *_Nonnull, int, int, Uint32);
	int (*_Nullable keyup)(void *_Nonnull, int, int, Uint32);
} VG_ToolOps;

/* VG tool instance */
typedef struct vg_tool {
	const VG_ToolOps *_Nonnull ops;		/* Class description */
	int selected;				/* Tool is in use? */
	int tag;				/* User tag */
	struct vg_view *_Nonnull vgv;		/* Associated view */
	void *_Nullable p;			/* User-supplied pointer */
	AG_Window *_Nullable editWin;		/* Edition window (if any) */
	AG_Widget *_Nullable editArea;		/* Edition area (if any) */
	VG_Vector vCursor;			/* Last cursor position */
	AG_SLIST_HEAD_(vg_tool_keybinding) kbindings;
	AG_TAILQ_HEAD_(vg_tool_command) cmds;
	AG_TAILQ_ENTRY(vg_tool) tools;
} VG_Tool;

/* General command handler */
typedef struct vg_tool_command {
	char     *_Nonnull  name;	/* Display name */
	char     *_Nullable descr;	/* Optional description */
	AG_Event *_Nonnull  fn;		/* Callback routine (in VG_View) */
	AG_KeyMod kMod;			/* Bound key modifier */
	AG_KeySym kSym;			/* Bound keysym */
	VG_Tool *_Nonnull tool;		/* Back pointer to tool */
	AG_TAILQ_ENTRY(vg_tool_command) cmds;
} VG_ToolCommand;

#define VGTOOL(t) ((VG_Tool *)(t))
#define VG_CURTOOL(vv) \
    (vv)->curtool != NULL ? (vv)->curtool : \
    (vv)->deftool != NULL ? (vv)->deftool : NULL

__BEGIN_DECLS
void VG_ToolInit(VG_Tool *_Nonnull);
void VG_ToolDestroy(VG_Tool *_Nonnull);

AG_Window *_Nonnull VG_ToolWindow(void *_Nonnull);

VG_ToolCommand *_Nonnull VG_ToolCommandNew(void *_Nonnull, const char *_Nonnull,
                                           _Nonnull AG_EventFn);
void VG_ToolCommandKey(VG_ToolCommand *_Nonnull, AG_KeyMod, AG_KeySym);
void VG_ToolCommandDescr(VG_ToolCommand *_Nonnull, const char *_Nonnull, ...);

int  VG_ToolCommandExec(void *_Nonnull, const char *_Nonnull,
                        const char *_Nullable, ...);
__END_DECLS

#include <agar/vg/close.h>
#endif /* _AGAR_VG_TOOL_H_ */
