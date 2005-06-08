/*	$Csoft: fill.h,v 1.5 2005/03/06 10:40:32 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_FILL_H_
#define _AGAR_RG_FILL_H_

#include <engine/rg/feature.h>

#include "begin_code.h"

struct rg_fill_feature {
	RG_Feature ft;

	enum fill_type {
		FILL_SOLID,
		FILL_HGRADIENT,
		FILL_VGRADIENT,
		FILL_CGRADIENT,
		FILL_PATTERN
	} type;
	Uint8 alpha;
	union {
		struct {
			Uint32 c;
		} solid;
		struct {
			Uint32 c1;
			Uint32 c2;
		} gradient;
		struct {
			int texid;
			int tex_xoffs;
			int tex_yoffs;
		} pattern;
	} args;
#define f_solid args.solid
#define f_gradient args.gradient
#define f_pattern args.pattern
};

__BEGIN_DECLS
void		 RG_FillInit(void *, RG_Tileset *, int);
int		 RG_FillLoad(void *, AG_Netbuf *);
void		 RG_FillSave(void *, AG_Netbuf *);
void		 RG_FillApply(void *, RG_Tile *, int, int);
void		 RG_FillMenu(void *, AG_MenuItem *);
AG_Window	*RG_FillEdit(void *, RG_Tileview *);
AG_Toolbar	*RG_FillToolbar(void *, RG_Tileview *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_FILL_H_ */
