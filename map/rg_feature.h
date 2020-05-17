/*	Public domain	*/

#ifndef _AGAR_RG_FEATURE_H_
#define _AGAR_RG_FEATURE_H_
#include <agar/map/begin.h>

#include <agar/gui/keyboard.h>

#define RG_FEATURE_NAME_MAX 32
#define RG_FEATURE_TYPE_MAX 32

struct rg_tileview;
struct ag_menu_item;
struct ag_toolbar;
struct ag_window;

typedef struct rg_feature_ops {
	const char *_Nonnull type;	/* Feature name */
#ifdef AG_HAVE_64BIT
	Uint64 len;             	/* Size of structure */
#else
	Uint len;	             	/* Size of structure */
#endif
	const char *_Nonnull desc;	/* Feature description */
	Uint flags;
#define FEATURE_AUTOREDRAW 0x01		/* Redraw tile periodically on edit */
	AG_KeyMod keymod;		/* Keyboard modifier */
	AG_KeySym keysym;		/* Keyboard shortcut (or NONE) */
	Uint rev;			/* Revision number */

	void (*_Nullable init)(void *_Nonnull, struct rg_tileset *_Nonnull, Uint);
	int  (*_Nullable load)(void *_Nonnull, AG_DataSource *_Nonnull);
	void (*_Nullable save)(void *_Nonnull, AG_DataSource *_Nonnull);
	void (*_Nullable destroy)(void *_Nonnull);
	void (*_Nullable apply)(void *_Nonnull, RG_Tile *_Nonnull, int,int);
	void (*_Nullable menu)(void *_Nonnull, struct ag_menu_item *_Nonnull);
	struct ag_toolbar *_Nonnull (*_Nullable toolbar)(void *_Nonnull,
	                                                 struct rg_tileview *_Nonnull);
	struct ag_window *_Nonnull (*_Nullable edit)(void *_Nonnull,
	                                             struct rg_tileview *_Nonnull);
} RG_FeatureOps;

#if 0
typedef struct rg_feature_sketch {
	struct rg_sketch *_Nonnull sk;
	int x, y;
	int visible;
	AG_TAILQ_ENTRY(rg_feature_sketch) sketches;
} RG_FeatureSketch;
#endif

typedef struct rg_feature_pixmap {
	struct rg_pixmap *_Nonnull px;
	Uint flags;
	int x, y;
	int visible;
	AG_TAILQ_ENTRY(rg_feature_pixmap) pixmaps;
} RG_FeaturePixmap;

typedef struct rg_feature {
	char name[RG_FEATURE_NAME_MAX];
	const RG_FeatureOps *_Nonnull ops;
	struct rg_tileset *_Nonnull ts;
	int flags;
	Uint nRefs;
/*	AG_TAILQ_HEAD_(rg_feature_sketch) sketches; */
	AG_TAILQ_HEAD_(rg_feature_pixmap) pixmaps;
	AG_TAILQ_ENTRY(rg_feature) features;
} RG_Feature;

#define RG_FEATURE(f) ((RG_Feature *)(f))

__BEGIN_DECLS
void RG_FeatureInit(void *_Nonnull, struct rg_tileset *_Nonnull, int,
                    const RG_FeatureOps *_Nonnull);
void RG_FeatureDestroy(RG_Feature *_Nonnull);
int  RG_FeatureLoad(void *_Nonnull, AG_DataSource *_Nonnull);
void RG_FeatureSave(void *_Nonnull, AG_DataSource *_Nonnull);

#if 0
RG_FeatureSketch *RG_FeatureAddSketch(RG_Feature *, struct rg_sketch *);
void		  RG_FeatureDelSketch(RG_Feature *, struct rg_sketch *);
#endif

RG_FeaturePixmap *_Nonnull RG_FeatureAddPixmap(RG_Feature *_Nonnull,
                                               struct rg_pixmap *_Nonnull);
void                       RG_FeatureDelPixmap(RG_Feature *_Nonnull,
                                               struct rg_pixmap *_Nonnull);

void RG_FeatureOpenMenu(struct rg_tileview *_Nonnull, int,int);
void RG_FeatureCloseMenu(struct rg_tileview *_Nonnull);
__END_DECLS

#include <agar/map/close.h>
#endif	/* _AGAR_RG_FEATURE_H_ */
