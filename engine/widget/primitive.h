/*	$Csoft: primitive.h,v 1.16 2002/12/31 10:31:54 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PRIMITIVE_H_
#define _AGAR_WIDGET_PRIMITIVE_H_
#include "begin_code.h"

struct primitive_ops {
	void	(*box)(void *p, int xoffs, int yoffs, int w, int h, int z,
		       Uint32 color);
	void	(*box_rect)(void *p, SDL_Rect *rd, int z, Uint32 color);
	void	(*frame)(void *p, int xoffs, int yoffs, int w, int h,
		         Uint32 color);
	void	(*frame_rect)(void *p, SDL_Rect *rd, Uint32 color);
	void	(*circle)(void *p, int xoffs, int yoffs, int w, int h,
		         int radius, Uint32 color);
	void	(*line)(void *p, int x1, int y1, int x2, int y2, Uint32 color);
	void	(*rect_outlined)(void *p, int x, int y, int w, int h,
		                 Uint32 color);
	void	(*rect_filled)(void *p, SDL_Rect *rd, Uint32 color);
};

extern struct primitive_ops primitives;

__BEGIN_DECLS
extern DECLSPEC struct window	*primitive_config_window(void);
extern DECLSPEC void		 primitives_init(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_PRIMITIVE_H_ */
