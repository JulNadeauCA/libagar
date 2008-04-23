/*	Public domain	*/

#ifndef _AGAR_RG_TRANSFORM_H_
#define _AGAR_RG_TRANSFORM_H_
#include "begin_code.h"

#define RG_TRANSFORM_MAX_ARGS	64	/* Max transform args */
#define RG_TRANSFORM_CHAIN_MAX	1000	/* Max transform chain entries */

enum rg_transform_type {
	RG_TRANSFORM_MIRROR,
	RG_TRANSFORM_FLIP,
	RG_TRANSFORM_ROTATE,
	RG_TRANSFORM_RGB_INVERT
};

typedef AG_TAILQ_HEAD(rg_transformq, rg_transform) RG_TransformChain;

typedef struct rg_transform {
	enum rg_transform_type type;
	AG_Surface *(*func)(AG_Surface *, int, Uint32 *);
	Uint32 *args;
	int nargs;
	AG_TAILQ_ENTRY(rg_transform) transforms;
} RG_Transform;

struct rg_transform_ops {
	char *name;
	enum rg_transform_type type;
	AG_Surface *(*func)(AG_Surface *, int, Uint32 *);
};

__BEGIN_DECLS
RG_Transform	*RG_TransformNew(enum rg_transform_type, int, Uint32 *);
int		 RG_TransformInit(RG_Transform *, enum rg_transform_type, int,
		                 Uint32 *);
int		 RG_TransformLoad(AG_DataSource *, RG_Transform *);
void		 RG_TransformSave(AG_DataSource *, const RG_Transform *);
void		 RG_TransformDestroy(RG_Transform *);

void	 RG_TransformChainInit(RG_TransformChain *);
int	 RG_TransformChainLoad(AG_DataSource *, RG_TransformChain *);
void	 RG_TransformChainSave(AG_DataSource *, const RG_TransformChain *);
void	 RG_TransformChainDestroy(RG_TransformChain *);
void	 RG_TransformChainPrint(const RG_TransformChain *, char *, size_t)
	                        BOUNDED_ATTRIBUTE(__string__, 2, 3);
void	 RG_TransformChainDup(const RG_TransformChain *, RG_TransformChain *);
int	 RG_TransformCompare(const RG_Transform *, const RG_Transform *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RG_TRANSFORM_H_ */
