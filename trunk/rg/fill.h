/*	Public domain	*/

#ifndef _AGAR_RG_FILL_H_
#define _AGAR_RG_FILL_H_
#include <agar/rg/begin.h>

struct ag_window;
struct ag_toolbar;

enum rg_fill_type {
	FILL_SOLID,
	FILL_HGRADIENT,
	FILL_VGRADIENT,
	FILL_CGRADIENT,
	FILL_PATTERN
};

struct rg_fill_feature {
	RG_Feature ft;
	enum rg_fill_type type;
	Uint8 alpha;
	union {
		struct {
			AG_Color c;
		} solid;
		struct {
			AG_Color c1;
			AG_Color c2;
		} gradient;
		struct {
			int texid;
			int tex_xoffs;
			int tex_yoffs;
		} pattern;
	} args;
#ifndef _AGAR_RG_PUBLIC_H_
#define f_solid args.solid
#define f_gradient args.gradient
#define f_pattern args.pattern
#endif
};

__BEGIN_DECLS
void		 RG_FillInit(void *, RG_Tileset *, int);
int		 RG_FillLoad(void *, AG_DataSource *);
void		 RG_FillSave(void *, AG_DataSource *);
void		 RG_FillApply(void *, RG_Tile *, int, int);
void		 RG_FillMenu(void *, AG_MenuItem *);
struct ag_window *RG_FillEdit(void *, RG_Tileview *);
struct ag_toolbar *RG_FillToolbar(void *, RG_Tileview *);
__END_DECLS

#include <agar/rg/close.h>
#endif /* _AGAR_RG_FILL_H_ */
