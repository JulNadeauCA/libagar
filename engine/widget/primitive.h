/*	$Csoft: primitive.h,v 1.30 2005/05/29 05:49:59 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PRIMITIVE_H_
#define _AGAR_WIDGET_PRIMITIVE_H_
#include "begin_code.h"

typedef struct ag_primitive_ops {
	void	(*box)(void *, int, int, int, int, int, Uint32);
	void	(*box_chamfered)(void *, SDL_Rect *, int, int, Uint32);
	void	(*frame)(void *, int, int, int, int, Uint32);
	void	(*frame_blended)(void *, int, int, int, int, Uint8 [4],
		                 enum ag_blend_func);
	void	(*circle)(void *, int, int, int, Uint32);
	void	(*circle2)(void *, int, int, int, Uint32);
	void	(*line)(void *, int, int, int, int, Uint32);
	void	(*line2)(void *, int, int, int, int, Uint32);
	void	(*line_blended)(void *, int, int, int, int, Uint8 [4],
		                enum ag_blend_func);
	void	(*hline)(void *, int, int, int, Uint32);
	void	(*vline)(void *, int, int, int, Uint32);
	void	(*rect_outlined)(void *, int, int, int, int, Uint32);
	void	(*rect_filled)(void *, int, int, int, int, Uint32);
	void	(*rect_blended)(void *, int, int, int, int, Uint8[4],
	                        enum ag_blend_func);
	void	(*plus)(void *, int, int, int, int, Uint8 [4],
			enum ag_blend_func);
	void	(*minus)(void *, int, int, int, int, Uint8 [4],
			 enum ag_blend_func);
	void	(*tiling)(void *, SDL_Rect, int, int, Uint32, Uint32);
} AG_PrimitiveOps;

extern AG_PrimitiveOps agPrim;

__BEGIN_DECLS
void AG_InitPrimitives(void);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_PRIMITIVE_H_ */
