/*	Public domain	*/

#ifndef _AGAR_CORE_DSO_H_
#define _AGAR_CORE_DSO_H_
#include <agar/core/begin.h>

#define AG_DSONAME_MAX 128

typedef struct ag_dso_sym {
	char *sym;
	void *p;
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
extern AG_Mutex agDSOLock;

AG_DSO *AG_LoadDSO(const char *, Uint);
int     AG_SymDSO(AG_DSO *, const char *, void **);
int     AG_UnloadDSO(AG_DSO *);
#define AG_LockDSO() AG_MutexLock(&agDSOLock)
#define AG_UnlockDSO() AG_MutexUnlock(&agDSOLock)
char  **AG_GetDSOList(Uint *);
void    AG_FreeDSOList(char **, Uint);

/* Return the named DSO or NULL if not found. */
static __inline__ AG_DSO *
AG_LookupDSO(const char *name)
{
	AG_DSO *dso;

	AG_LockDSO();
	AG_TAILQ_FOREACH(dso, &agLoadedDSOs, dsos) {
		if (strcmp(dso->name, name) == 0)
			break;
	}
	AG_UnlockDSO();
	return (dso);
}
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_DSO_H_ */
