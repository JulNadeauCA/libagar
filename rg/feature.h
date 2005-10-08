/*	$Csoft: feature.h,v 1.9 2005/03/05 12:13:49 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_FEATURE_H_
#define _AGAR_RG_FEATURE_H_
#include "begin_code.h"

#define RG_FEATURE_NAME_MAX 32
#define RG_FEATURE_TYPE_MAX 32

struct rg_tileview;
struct ag_menu_item;

typedef struct rg_feature_ops {
	const char *type;
	size_t len;
	const char *desc;
	int flags;
#define FEATURE_AUTOREDRAW	0x01	/* Redraw tile periodically on edit */

	void (*init)(void *, struct rg_tileset *, int);
	int  (*load)(void *, AG_Netbuf *);
	void (*save)(void *, AG_Netbuf *);
	void (*destroy)(void *);
	void (*apply)(void *, RG_Tile *, int, int);
	void (*menu)(void *, struct ag_menu_item *);
	AG_Toolbar *(*toolbar)(void *, struct rg_tileview *);
	AG_Window *(*edit)(void *, struct rg_tileview *);
} RG_FeatureOps;

typedef struct rg_feature_sketch {
	struct rg_sketch *sk;
	int x, y;
	int visible;
	TAILQ_ENTRY(rg_feature_sketch) sketches;
} RG_FeatureSketch;

typedef struct rg_feature_pixmap {
	struct rg_pixmap *px;
	int x, y;
	int visible;
	TAILQ_ENTRY(rg_feature_pixmap) pixmaps;
} RG_FeaturePixmap;

typedef struct rg_feature {
	char name[RG_FEATURE_NAME_MAX];
	const RG_FeatureOps *ops;
	struct rg_tileset *ts;
	int flags;
	u_int nrefs;
	TAILQ_HEAD(,rg_feature_sketch) sketches;
	TAILQ_HEAD(,rg_feature_pixmap) pixmaps;
	TAILQ_ENTRY(rg_feature) features;
} RG_Feature;

#define RG_FEATURE(f) ((RG_Feature *)(f))

__BEGIN_DECLS
void	AG_FeatureInit(void *, struct rg_tileset *, int, const RG_FeatureOps *);
void	AG_FeatureDestroy(RG_Feature *);
int	RG_FeatureLoad(void *, AG_Netbuf *);
void	RG_FeatureSave(void *, AG_Netbuf *);

RG_FeatureSketch *RG_FeatureAddSketch(RG_Feature *, struct rg_sketch *);
void		  RG_FeatureDelSketch(RG_Feature *, struct rg_sketch *);

RG_FeaturePixmap *RG_FeatureAddPixmap(RG_Feature *, struct rg_pixmap *);
void		  RG_FeatureDelPixmap(RG_Feature *, struct rg_pixmap *);

void	RG_FeatureOpenMenu(struct rg_tileview *, int, int);
void	RG_FeatureCloseMenu(struct rg_tileview *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_FEATURE_H_ */
