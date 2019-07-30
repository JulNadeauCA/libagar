/*	Public domain	*/

#include <agar/map/begin.h>

#ifndef AG_MAPTOOL_STATUS_MAX
#define AG_MAPTOOL_STATUS_MAX 8
#endif

struct ag_widget;
struct ag_window;
struct ag_button;

struct map_view;
struct map_tool_keybinding;
struct map_tool_mousebinding;

typedef struct map_tool_ops {
	const char    *_Nonnull name;	/* Display name */
	const char    *_Nonnull desc;	/* Long description */
	AG_StaticIcon *_Nonnull icon;	/* Display icon */
#ifdef AG_HAVE_64BIT
	Uint64 len;
#else
	Uint len;
#endif
	Uint flags;
#define TOOL_HIDDEN	0x01		/* Don't include in toolbars/menus */
	int rev;

	void (*_Nullable init)(void *_Nonnull);
	void (*_Nullable destroy)(void *_Nonnull);
	void (*_Nullable edit_pane)(void *_Nonnull, void *_Nonnull);
	void (*_Nullable edit)(void *_Nonnull);
	int  (*_Nullable cursor)(void *_Nonnull, AG_Rect *_Nonnull);
	int  (*_Nullable effect)(void *_Nonnull, MAP_Node *_Nonnull);
	int  (*_Nullable mousemotion)(void *_Nonnull, int,int, int,int, int);
	int  (*_Nullable mousebuttondown)(void *_Nonnull, int,int, int);
	int  (*_Nullable mousebuttonup)(void *_Nonnull, int,int, int);
	int  (*_Nullable keydown)(void *_Nonnull, int, int);
	int  (*_Nullable keyup)(void *_Nonnull, int, int);
} MAP_ToolOps;

typedef struct map_tool {
	const MAP_ToolOps *_Nonnull ops;

	struct map_view *_Nullable mv;		/* Associated view */
	void *_Nullable p;			/* User pointer */

	char *_Nullable status[AG_MAPTOOL_STATUS_MAX];  /* Status */
	int            nstatus;
	Uint32 _pad;
	struct ag_window *_Nullable win;	/* Edition window (if any) */
	struct ag_widget *_Nullable pane;	/* Edition pane (if any) */
	struct ag_button *_Nullable trigger;	/* Trigger button (XXX) */

	AG_SLIST_HEAD_(map_tool_keybinding) kbindings;
	AG_SLIST_HEAD_(map_tool_mousebinding) mbindings;
	AG_TAILQ_ENTRY(map_tool) tools;
} MAP_Tool;

typedef struct map_tool_keybinding {
	AG_KeyMod mod;
	AG_KeySym key;
	int edit;
	Uint32 _pad;
	int (*_Nonnull func)(MAP_Tool *_Nonnull, AG_KeySym k, int s,
	                     void *_Nullable);
	void *_Nullable arg;
	AG_SLIST_ENTRY(map_tool_keybinding) kbindings;
} MAP_ToolKeyBinding;

typedef struct map_tool_mousebinding {
	int button;
	int edit;
	int (*_Nonnull func)(MAP_Tool *_Nonnull, int b, int s, int x, int y,
	                     void *_Nullable);
	void *_Nullable arg;
	AG_SLIST_ENTRY(map_tool_mousebinding) mbindings;
} MAP_ToolMouseBinding;

#define TOOL(t) ((MAP_Tool *)(t))

__BEGIN_DECLS
void MAP_ToolInit(MAP_Tool *_Nonnull);
void MAP_ToolDestroy(MAP_Tool *_Nonnull);

struct ag_window *_Nullable MAP_ToolWindow(void *_Nonnull, const char *_Nonnull);

void MAP_ToolBindKey(void *_Nonnull, AG_KeyMod, AG_KeySym,
                     int (*_Nonnull)(MAP_Tool *_Nonnull, AG_KeySym, int, void *_Nullable),
		     void *_Nullable);

void MAP_ToolBindMouseButton(void *_Nonnull, int,
                             int (*_Nonnull)(MAP_Tool *_Nonnull, int, int,
			                     int,int, void *_Nullable),
                             void *_Nullable);

void MAP_ToolUnbindKey(void *_Nonnull, AG_KeyMod, AG_KeySym);

void MAP_ToolPushStatus(void *_Nonnull, const char *_Nonnull, ...);
void MAP_ToolSetStatus(void *_Nonnull, const char *_Nonnull, ...);
void MAP_ToolPopStatus(void *_Nonnull);
void MAP_ToolUpdateStatus(void *_Nonnull);
__END_DECLS

#include <agar/map/close.h>
