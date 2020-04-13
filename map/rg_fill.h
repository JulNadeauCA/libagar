/*	Public domain	*/

#ifndef _AGAR_RG_FILL_H_
#define _AGAR_RG_FILL_H_
#include <agar/map/begin.h>

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
	Uint8 _pad1[7];
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
#define f_solid    args.solid
#define f_gradient args.gradient
#define f_pattern  args.pattern
#endif
#if AG_MODEL == AG_LARGE
	Uint32 _pad2;
#endif
};

__BEGIN_DECLS
void RG_FillInit(void *_Nonnull, RG_Tileset *_Nonnull, Uint);
int  RG_FillLoad(void *_Nonnull, AG_DataSource *_Nonnull);
void RG_FillSave(void *_Nonnull, AG_DataSource *_Nonnull);
void RG_FillApply(void *_Nonnull, RG_Tile *_Nonnull, int,int);
void RG_FillMenu(void *_Nonnull, AG_MenuItem *_Nonnull);
struct ag_window *_Nonnull RG_FillEdit(void *_Nonnull, RG_Tileview *_Nonnull);
struct ag_toolbar *_Nonnull RG_FillToolbar(void *_Nonnull, RG_Tileview *_Nonnull);
__END_DECLS

#include <agar/map/close.h>
#endif /* _AGAR_RG_FILL_H_ */
