/*	$Csoft: primitive.h,v 1.20 2003/05/26 03:03:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PRIMITIVE_H_
#define _AGAR_WIDGET_PRIMITIVE_H_
#include "begin_code.h"

struct primitive_ops {
	void	(*box)(void *, int, int, int, int, int, Uint32);
	void	(*frame)(void *, int, int, int, int, Uint32);
	void	(*circle)(void *, int, int, int, int, int, Uint32);
	void	(*line)(void *, int, int, int, int, Uint32);
	void	(*line2)(void *, int, int, int, int, Uint32);
	void	(*rect_outlined)(void *, int, int, int, int, Uint32);
	void	(*rect_filled)(void *, int, int, int, int, Uint32);
	void	(*plus)(void *, int, int, int, int, Uint32);
	void	(*minus)(void *, int, int, int, int, Uint32);
};

extern struct primitive_ops primitives;

__BEGIN_DECLS
extern DECLSPEC struct window	*primitive_config_window(void);
extern DECLSPEC void		 primitives_init(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_PRIMITIVE_H_ */
