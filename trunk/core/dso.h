/*	Public domain	*/

#include "begin_code.h"

#define AG_DSONAME_MAX 128

typedef struct ag_dso_sym {
	char *sym;
	void *p;
	AG_TAILQ_ENTRY(ag_dso_sym) syms;
} AG_DSOSym;

typedef struct ag_dso {
	char name[AG_DSONAME_MAX];		/* Module name */
	char path[AG_PATHNAME_MAX];		/* Path to DSO */
	Uint flags;
	AG_TAILQ_HEAD(,ag_dso_sym) syms;	/* Previously used symbols */
	AG_TAILQ_ENTRY(ag_dso) dsos;
} AG_DSO;

AG_TAILQ_HEAD(ag_dsoq, ag_dso);

#define AGDSO(p) ((AG_DSO *)(p))

__BEGIN_DECLS
extern struct ag_dsoq agLoadedDSOs;
#ifdef THREADS
extern AG_Mutex agDSOLock;
#endif

AG_DSO *AG_LoadDSO(const char *, const char *, Uint);
int     AG_SymDSO(AG_DSO *, const char *, void **);
int     AG_UnloadDSO(AG_DSO *);
__END_DECLS

#include "close_code.h"
