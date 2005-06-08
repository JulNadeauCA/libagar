/*	$Csoft: separator.h,v 1.1 2005/02/19 09:28:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SEPARATOR_H_
#define _AGAR_WIDGET_SEPARATOR_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

enum ag_separator_type {
	AG_SEPARATOR_HORIZ,
	AG_SEPARATOR_VERT
};

typedef struct ag_separator {
	struct ag_widget wid;
	enum ag_separator_type type;
} AG_Separator;

__BEGIN_DECLS
AG_Separator *AG_SeparatorNew(void *, enum ag_separator_type);
void	      AG_SeparatorInit(AG_Separator *, enum ag_separator_type);
void	      AG_SeparatorDraw(void *);
void	      AG_SeparatorScale(void *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_SEPARATOR_H_ */
