/*	Public domain	*/

#ifndef _AGAR_CORE_DSO_H_
#define _AGAR_CORE_DSO_H_

#ifdef AG_ENABLE_DSO
#include <agar/core/begin.h>

#ifndef AG_DSONAME_MAX
#define AG_DSONAME_MAX AG_MODEL
#endif

typedef struct ag_dso_sym {
	char *_Nonnull sym;              /* Symbol name */
	void *_Nullable p;               /* Address */
	AG_TAILQ_ENTRY(ag_dso_sym) syms;
} AG_DSOSym;

typedef struct ag_dso {
	char name[AG_DSONAME_MAX];		/* Module name */
	char path[AG_PATHNAME_MAX];		/* Path to DSO */
	Uint refCount;				/* Reference count */
	Uint flags;
	AG_TAILQ_HEAD_(ag_dso_sym) syms;	/* Previously used symbols */
	AG_TAILQ_ENTRY(ag_dso) dsos;
} AG_DSO;

AG_TAILQ_HEAD(ag_dsoq, ag_dso);
#define AGDSO(p) ((AG_DSO *)(p))

__BEGIN_DECLS
extern struct ag_dsoq agLoadedDSOs;
extern _Nonnull_Mutex AG_Mutex agDSOLock;

AG_DSO *_Nullable AG_LookupDSO(const char *_Nonnull)
                              _Pure_Attribute_If_Unthreaded;

AG_DSO *_Nullable AG_LoadDSO(const char *_Nonnull, Uint);

int AG_SymDSO(AG_DSO *_Nonnull, const char *_Nonnull, void *_Nonnull *_Nullable);
int AG_UnloadDSO(AG_DSO *_Nonnull);

#define AG_LockDSO()   AG_MutexLock(&agDSOLock)
#define AG_UnlockDSO() AG_MutexUnlock(&agDSOLock)

char *_Nonnull *_Nullable AG_GetDSOList(Uint *_Nonnull);
void                      AG_FreeDSOList(char *_Nonnull *_Nullable, Uint);
__END_DECLS

#include <agar/core/close.h>
#endif /* AG_ENABLE_DSO */
#endif /* _AGAR_CORE_DSO_H_ */
