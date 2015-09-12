/*	Public domain	*/

#ifndef _AGAR_GUI_STYLESHEET_H_
#define _AGAR_GUI_STYLESHEET_H_
#include <agar/gui/begin.h>

#define AG_STYLE_VALUE_MAX 128

typedef struct ag_style_entry {
	char key[AG_VARIABLE_NAME_MAX];			/* Target parameter */
	char value[AG_STYLE_VALUE_MAX];			/* Set value */
	AG_TAILQ_ENTRY(ag_style_entry) ents;
} AG_StyleEntry;

typedef struct ag_style_block {
	char match[64];					/* Pattern */
	AG_TAILQ_HEAD_(ag_style_entry) ents;		/* Entries in block */
	AG_TAILQ_ENTRY(ag_style_block) blks;
} AG_StyleBlock;

typedef struct ag_style_sheet {
	AG_TAILQ_HEAD_(ag_style_block) blks;		/* By widget class */
} AG_StyleSheet;

/* Description of a built-in stylesheet. */
typedef struct ag_static_css {
	const char *name;		/* Identifier */
	Uint32 size;			/* Size in bytes */
	const char **data;		/* CSS data */
	AG_StyleSheet *css;		/* Initialized stylesheet */
} AG_StaticCSS;

__BEGIN_DECLS
extern AG_StyleSheet agDefaultCSS;

extern AG_StaticCSS agStyleDefault;

void           AG_InitStyleSheet(AG_StyleSheet *);
void           AG_DestroyStyleSheet(AG_StyleSheet *);
AG_StyleSheet *AG_LoadStyleSheet(void *, const char *);
int            AG_LookupStyleSheet(AG_StyleSheet *, void *, const char *, char **);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_STYLESHEET_H_ */
