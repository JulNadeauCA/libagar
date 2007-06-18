/*	Public domain	*/

#ifndef _AGAR_SK_TOOL_H_
#define _AGAR_SK_TOOL_H_

struct sk_tool_keybinding;
struct sk_tool_mousebinding;
struct sk_view;

typedef struct sk_tool_ops {
	const char *name;
	const char *desc;
	int icon;
	size_t len;
	Uint flags;
#define SK_MOUSEMOTION_NOSNAP	0x01	/* Ignore snapping in mousemotion */
#define SK_BUTTONUP_NOSNAP	0x02	/* Ignore snapping in buttonup */
#define SK_BUTTONDOWN_NOSNAP	0x04	/* Ignore snapping in buttondown */
#define SK_BUTTON_NOSNAP	(SK_BUTTONUP_NOSNAP|SK_BUTTONDOWN_NOSNAP)
#define SK_NOSNAP		(SK_BUTTON_NOSNAP|SK_MOUSEMOTION_NOSNAP)

	void (*init)(void *);
	void (*destroy)(void *);
	void (*edit)(void *, void *);
	int (*mousemotion)(void *, SG_Vector pos, SG_Vector vel, int btn);
	int (*mousebuttondown)(void *, SG_Vector pos, int btn);
	int (*mousebuttonup)(void *, SG_Vector pos, int btn);
	int (*keydown)(void *, int ksym, int kmod);
	int (*keyup)(void *, int ksym, int kmod);
} SK_ToolOps;

typedef struct sk_tool {
	const SK_ToolOps *ops;
	struct sk_view *skv;			/* Associated view */
	void *p;				/* User-supplied pointer */
	AG_Window *win;				/* Edition window (if any) */
	AG_Widget *pane;			/* Edition pane (if any) */
	AG_Button *trigger;			/* Trigger button (XXX) */
	SLIST_HEAD(,sk_tool_keybinding) kbindings;
	SLIST_HEAD(,sk_tool_mousebinding) mbindings;
	TAILQ_ENTRY(sk_tool) tools;
} SK_Tool;

typedef struct sk_tool_keybinding {
	SDLMod mod;
	SDLKey key;
	int edit;
	int (*func)(SK_Tool *, SDLKey k, int s, void *);
	void *arg;
	SLIST_ENTRY(sk_tool_keybinding) kbindings;
} SK_ToolKeyBinding;

typedef struct sk_tool_mousebinding {
	int button;
	int edit;
	int (*func)(SK_Tool *, int b, int s, SG_Vector pos, void *);
	void *arg;
	SLIST_ENTRY(sk_tool_mousebinding) mbindings;
} SK_ToolMouseBinding;

#define SKTOOL(t) ((SK_Tool *)(t))
#define SK_CURTOOL(skv) \
    (skv)->curtool != NULL ? (skv)->curtool : \
    (skv)->deftool != NULL ? (skv)->deftool : NULL

__BEGIN_DECLS
void		 SK_ToolInit(SK_Tool *);
void		 SK_ToolDestroy(SK_Tool *);
AG_Window	*SK_ToolWindow(void *, const char *);

void SK_ToolBindKey(void *, SDLMod, SDLKey,
		    int (*)(SK_Tool *, SDLKey, int, void *), void *);
void SK_ToolBindMouseButton(void *, int,
			    int (*)(SK_Tool *, int, int, SG_Vector, void *),
			    void *);
void SK_ToolUnbindKey(void *, SDLMod, SDLKey);
__END_DECLS

#endif /* _AGAR_SK_TOOL_H_ */
