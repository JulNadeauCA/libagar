/*	$Csoft: feature.h,v 1.8 2005/03/03 10:51:01 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RG_FEATURE_H_
#define _AGAR_RG_FEATURE_H_
#include "begin_code.h"

#define FEATURE_NAME_MAX 32
#define FEATURE_TYPE_MAX 32

struct tileview;

struct feature_ops {
	const char *type;
	size_t len;
	const char *desc;
	int flags;
#define FEATURE_AUTOREDRAW	0x01	/* Redraw tile periodically on edit */

	void (*init)(void *, struct tileset *, int);
	int  (*load)(void *, struct netbuf *);
	void (*save)(void *, struct netbuf *);
	void (*destroy)(void *);
	void (*apply)(void *, struct tile *, int, int);
	void (*menu)(void *, struct AGMenuItem *);
	struct toolbar *(*toolbar)(void *, struct tileview *);
	struct window *(*edit)(void *, struct tileview *);
};

struct feature_sketch {
	struct sketch *sk;
	int x, y;
	int visible;
	TAILQ_ENTRY(feature_sketch) sketches;
};

struct feature_pixmap {
	struct pixmap *px;
	int x, y;
	int visible;
	TAILQ_ENTRY(feature_pixmap) pixmaps;
};

struct feature {
	char name[FEATURE_NAME_MAX];
	const struct feature_ops *ops;
	struct tileset *ts;
	int flags;
	u_int nrefs;

	TAILQ_HEAD(,feature_sketch) sketches;
	TAILQ_HEAD(,feature_pixmap) pixmaps;
	TAILQ_ENTRY(feature) features;
};

#define FEATURE(f) ((struct feature *)(f))

__BEGIN_DECLS
void	feature_init(void *, struct tileset *, int, const struct feature_ops *);
void	feature_destroy(struct feature *);
int	feature_load(void *, struct netbuf *);
void	feature_save(void *, struct netbuf *);

struct feature_sketch *feature_insert_sketch(struct feature *, struct sketch *);
void		       feature_remove_sketch(struct feature *, struct sketch *);

struct feature_pixmap *feature_insert_pixmap(struct feature *, struct pixmap *);
void		       feature_remove_pixmap(struct feature *, struct pixmap *);

void	feature_open_menu(struct tileview *, int, int);
void	feature_close_menu(struct tileview *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_FEATURE_H_ */
