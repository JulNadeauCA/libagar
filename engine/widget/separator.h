/*	$Csoft: separator.h,v 1.31 2005/01/30 05:39:11 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_SEPARATOR_H_
#define _AGAR_WIDGET_SEPARATOR_H_
#include <engine/widget/widget.h>
#include "begin_code.h"

enum separator_type {
	SEPARATOR_HORIZ,
	SEPARATOR_VERT
};

struct separator {
	struct widget wid;
	enum separator_type type;
};

__BEGIN_DECLS
struct separator *separator_new(void *, enum separator_type);
void	 	  separator_init(struct separator *, enum separator_type);
void	 	  separator_destroy(void *);
void		  separator_draw(void *);
void		  separator_scale(void *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_SEPARATOR_H_ */
