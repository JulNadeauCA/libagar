/*	$Csoft: fill.h,v 1.2 2005/01/26 02:43:03 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_FILL_H_
#define _AGAR_RG_FILL_H_

#include <engine/rg/feature.h>

#include "begin_code.h"

struct fill {
	struct feature ft;

	enum fill_type {
		FILL_SOLID,
		FILL_HGRADIENT,
		FILL_VGRADIENT,
		FILL_CGRADIENT,
		FILL_PATTERN
	} type;
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
void		 fill_init(void *, struct tileset *, int);
int		 fill_load(void *, struct netbuf *);
void		 fill_save(void *, struct netbuf *);
void		 fill_apply(void *, struct tile *, int, int);
void		 fill_menu(void *, struct AGMenuItem *);
struct window	*fill_edit(void *, struct tileview *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_RG_FILL_H_ */
