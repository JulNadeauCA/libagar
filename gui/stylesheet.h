/*	Public domain	*/

#ifndef _AGAR_GUI_STYLESHEET_H_
#define _AGAR_GUI_STYLESHEET_H_
#include <agar/gui/begin.h>

#ifndef AG_STYLE_VALUE_MAX
#define AG_STYLE_VALUE_MAX (AG_MODEL*2 + 4)
#endif

typedef struct ag_style_entry {
	char key[AG_VARIABLE_NAME_MAX];  /* Target parameter */
	char value[AG_STYLE_VALUE_MAX];  /* Set value */
	AG_TAILQ_ENTRY(ag_style_entry) ents;
} AG_StyleEntry;

enum ag_style_selector_type {
	AG_SELECTOR_CLASS_NAME,         /* Widgets of class E (exact match) */
	AG_SELECTOR_CLASS_PATTERN,      /* Widgets of class E (pattern match) */
	AG_SELECTOR_CHILD_NAMED,        /* Widget under E named F */
	AG_SELECTOR_CHILD_OF_CLASS,     /* Widget under E of class F */
	AG_SELECTOR_LAST
};

enum ag_style_condition_type {
	AG_SELECTOR_COND_NONE,         /* No condition */
	AG_SELECTOR_COND_WIDTH,        /* width x-y */
	AG_SELECTOR_COND_HEIGHT,       /* height x-y */
	AG_SELECTOR_COND_ZOOM,         /* zoom x-y */
	AG_SELECTOR_COND_LAST
};

/* Stylesheet element. */
typedef struct ag_style_block {
	enum ag_style_selector_type selector;   /* Type of selector */
	enum ag_style_condition_type cond;      /* Conditional */
	int x, y;                               /* Conditional constants */
	char e[64];                             /* Class name or pattern (E) */
	char f[32];                             /* Child name or class (F) */
	AG_TAILQ_HEAD_(ag_style_entry) ents;    /* Entries in block */
	AG_TAILQ_ENTRY(ag_style_block) blks;
} AG_StyleBlock;

typedef struct ag_style_sheet {
	AG_TAILQ_HEAD_(ag_style_block) blks;     /* Blocks with no condition */
	AG_TAILQ_HEAD_(ag_style_block) blksCond; /* Blocks with condition */
} AG_StyleSheet;

/* Built-in Agar stylesheet */
typedef struct ag_static_css {
	const char *_Nonnull name;              /* Identifier */
#ifdef AG_HAVE_64BIT
	Uint64 size;                            /* Length (bytes) */
#else
	Uint size;                              /* Length (bytes) */
#endif
	const char *_Nonnull *_Nonnull data;    /* Minified CSS */
	AG_StyleSheet *_Nullable css;           /* Initialized stylesheet */
} AG_StaticCSS;

__BEGIN_DECLS
extern AG_StyleSheet agDefaultCSS;
extern AG_StaticCSS agStyleDefault;

void AG_InitStyleSheet(AG_StyleSheet *_Nonnull);
void AG_DestroyStyleSheet(AG_StyleSheet *_Nonnull);

AG_StyleSheet *_Nullable AG_LoadStyleSheet(void *_Nullable, const char *_Nonnull);

int AG_LookupStyleSheet(AG_StyleSheet *_Nonnull, void *_Nonnull,
                        const char *_Nonnull, char *_Nonnull *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_STYLESHEET_H_ */
