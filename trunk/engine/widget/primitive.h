/*	$Csoft: primitive.h,v 1.27 2005/03/08 08:39:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PRIMITIVE_H_
#define _AGAR_WIDGET_PRIMITIVE_H_
#include "begin_code.h"

struct primitive_ops {
	void	(*box)(void *, int, int, int, int, int, Uint32);
	void	(*box_chamfered)(void *, SDL_Rect *, int, int, Uint32);
	void	(*frame)(void *, int, int, int, int, Uint32);
	void	(*circle)(void *, int, int, int, Uint32);
	void	(*circle2)(void *, int, int, int, Uint32);
	void	(*line)(void *, int, int, int, int, Uint32);
	void	(*line2)(void *, int, int, int, int, Uint32);
	void	(*rect_outlined)(void *, int, int, int, int, Uint32);
	void	(*rect_filled)(void *, int, int, int, int, Uint32);
	void	(*plus)(void *, int, int, int, int, Uint32);
	void	(*minus)(void *, int, int, int, int, Uint32);
	void	(*tiling)(void *, SDL_Rect, int, int, Uint32, Uint32);
};

extern struct primitive_ops primitives;

__BEGIN_DECLS
struct window	*primitive_config_window(void);
void		 primitives_init(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_PRIMITIVE_H_ */
