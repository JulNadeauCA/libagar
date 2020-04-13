/*	Public domain	*/

#ifndef _AGAR_RG_TRANSFORM_H_
#define _AGAR_RG_TRANSFORM_H_
#include <agar/map/begin.h>

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
	Uint             nArgs;
	Uint32 *_Nullable args;
	AG_Surface *_Nonnull (*_Nullable func)(AG_Surface *_Nonnull, int,
	                                       Uint32 *_Nonnull);
	AG_TAILQ_ENTRY(rg_transform) transforms;
} RG_Transform;

struct rg_transform_ops {
	char *_Nonnull name;
	enum rg_transform_type type;
	Uint flags;
	AG_Surface *_Nonnull (*_Nonnull func)(AG_Surface *_Nonnull, int,
	                                      Uint32 *_Nonnull);
};

__BEGIN_DECLS
RG_Transform *_Nullable RG_TransformNew(enum rg_transform_type, int,
                                        Uint32 *_Nullable);
int  RG_TransformInit(RG_Transform *_Nonnull, enum rg_transform_type, Uint,
                      Uint32 *_Nullable);
int  RG_TransformLoad(AG_DataSource *_Nonnull, RG_Transform *_Nonnull);
void RG_TransformSave(AG_DataSource *_Nonnull, const RG_Transform *_Nonnull);
void RG_TransformDestroy(RG_Transform *_Nonnull);

void RG_TransformChainInit(RG_TransformChain *_Nonnull);
int  RG_TransformChainLoad(AG_DataSource *_Nonnull, RG_TransformChain *_Nonnull);
void RG_TransformChainSave(AG_DataSource *_Nonnull, const RG_TransformChain *_Nonnull);
void RG_TransformChainDestroy(RG_TransformChain *_Nonnull);
void RG_TransformChainPrint(const RG_TransformChain *_Nonnull, char *_Nonnull, AG_Size);
void RG_TransformChainDup(const RG_TransformChain *_Nonnull,
                          RG_TransformChain *_Nonnull);
int  RG_TransformCompare(const RG_Transform *_Nonnull,
                         const RG_Transform *_Nonnull);
__END_DECLS

#include <agar/map/close.h>
#endif	/* _AGAR_RG_TRANSFORM_H_ */
