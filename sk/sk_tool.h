/*	Public domain	*/

struct sk_tool_keybinding;
struct sk_tool_mousebinding;
struct sk_view;
struct ag_static_icon;

typedef struct sk_tool_ops {
	const char *_Nonnull name;		/* Short name */
	const char *_Nonnull desc;		/* Tool description */
	struct ag_static_icon *_Nullable icon;	/* Optional icon */
	AG_Size len;
#if AG_MODEL == AG_LARGE
	Uint64 flags;
#else
	Uint flags;
#endif
#define SK_MOUSEMOTION_NOSNAP	0x01	/* Ignore snapping in mousemotion */
#define SK_BUTTONUP_NOSNAP	0x02	/* Ignore snapping in buttonup */
#define SK_BUTTONDOWN_NOSNAP	0x04	/* Ignore snapping in buttondown */
#define SK_BUTTON_NOSNAP	(SK_BUTTONUP_NOSNAP|SK_BUTTONDOWN_NOSNAP)
#define SK_NOSNAP		(SK_BUTTON_NOSNAP|SK_MOUSEMOTION_NOSNAP)

	void (*_Nullable init)(void *_Nonnull);
	void (*_Nullable destroy)(void *_Nonnull);
	void (*_Nullable edit)(void *_Nonnull, void *_Nonnull);
	int  (*_Nullable mousemotion)(void *_Nonnull, M_Vector3 pos,
	                              M_Vector3 vel, int btn);
	int  (*_Nullable mousebuttondown)(void *_Nonnull, M_Vector3 pos, int btn);
	int  (*_Nullable mousebuttonup)(void *_Nonnull, M_Vector3 pos, int btn);
	int  (*_Nullable keydown)(void *_Nonnull, int ksym, int kmod);
	int  (*_Nullable keyup)(void *_Nonnull, int ksym, int kmod);
} SK_ToolOps;

typedef struct sk_tool {
	const SK_ToolOps *_Nonnull ops;
	struct sk_view *_Nullable skv;		/* Associated view */
	void *_Nullable p;			/* User-supplied pointer */
	AG_Window *_Nullable win;		/* Edition window (if any) */
	AG_Button *_Nullable trigger;		/* Trigger button (XXX) */
	AG_SLIST_HEAD_(sk_tool_keybinding) kbindings;
	AG_SLIST_HEAD_(sk_tool_mousebinding) mbindings;
	AG_TAILQ_ENTRY(sk_tool) tools;
} SK_Tool;

typedef struct sk_tool_keybinding {
	AG_KeyMod mod;
	AG_KeySym key;
	int edit;
	Uint32 _pad;
	int (*_Nonnull func)(SK_Tool *_Nonnull, AG_KeySym k, int s,
	                     void *_Nullable);
	void *_Nullable arg;
	AG_SLIST_ENTRY(sk_tool_keybinding) kbindings;
} SK_ToolKeyBinding;

typedef struct sk_tool_mousebinding {
	int button;
	int edit;
	int (*_Nonnull func)(SK_Tool *_Nonnull, int b, int s, M_Vector3 pos,
	                     void *_Nullable);
	void *_Nullable arg;
	AG_SLIST_ENTRY(sk_tool_mousebinding) mbindings;
} SK_ToolMouseBinding;

#define SKTOOL(t) ((SK_Tool *)(t))
#define SK_CURTOOL(skv) \
    (skv)->curtool != NULL ? (skv)->curtool : \
    (skv)->deftool != NULL ? (skv)->deftool : NULL

__BEGIN_DECLS
void SK_ToolInit(SK_Tool *_Nonnull);
void SK_ToolDestroy(SK_Tool *_Nonnull);

AG_Window *_Nonnull SK_ToolWindow(void *_Nonnull, const char *_Nonnull);

void SK_ToolBindKey(void *_Nonnull, AG_KeyMod, AG_KeySym,
		    int (*_Nonnull)(SK_Tool *_Nonnull, AG_KeySym, int, void *_Nullable),
		    void *_Nullable);

void SK_ToolBindMouseButton(void *_Nonnull, int,
			    int (*_Nonnull)(SK_Tool *_Nonnull, int,int,
			                    M_Vector3, void *_Nullable),
			    void *_Nullable);

void SK_ToolUnbindKey(void *_Nonnull, AG_KeyMod, AG_KeySym);
__END_DECLS
