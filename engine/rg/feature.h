/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_BG_FEATURE_H_
#define _AGAR_BG_FEATURE_H_
#include "begin_code.h"

#define FEATURE_NAME_MAX 32

struct feature_ops {
	const char *name;
	const char *desc;

	void (*init)(void *);
	void (*destroy)(void *);
	void (*apply)(void *);
	void (*edit)(void *);
};

struct feature_sketch {
	struct sketch *sk;
	int x, y;
	int visible, suppressed;
	TAILQ_ENTRY(feature_sketch) sketches;
};

struct feature {
	char name[FEATURE_NAME_MAX];
	enum feature_type type;
	TAILQ_HEAD(,feature_sketch) sketches;
	TAILQ_ENTRY(feature) features;
};

__BEGIN_DECLS
void	feature_init(struct feature *, enum feature_type, const char *);
void	feature_destroy(struct feature *);

struct feature_sketch *feature_insert_sketch(struct feature *, struct sketch *);
void		       feature_remove_sketch(struct feature *, struct sketch *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_BG_FEATURE_H_ */
