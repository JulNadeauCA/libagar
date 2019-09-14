/*	Public domain	*/

#ifndef M_BEZIER_PRIMITIVES_H
#define M_BEZIER_PRIMITIVES_H

#include <agar/math/begin.h>

__BEGIN_DECLS

void M_DrawBezier2(AG_Widget*, M_Real, M_Real, M_Real, M_Real, M_Real, 
		M_Real, M_Real, M_Real, int, AG_Color*);

__END_DECLS

#include <agar/math/close.h>

#endif /* M_BEZIER_PRIMITIVES_H */
