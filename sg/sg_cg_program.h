/*	Public domain	*/

#ifndef _AGAR_SG_CG_PROGRAM_H_
#define _AGAR_SG_CG_PROGRAM_H_

#include <agar/config/have_cg.h>
#ifdef HAVE_CG

#include <Cg/cg.h>
#include <Cg/cgGL.h>

typedef struct sg_cg_program {
	struct sg_program _inherit;	/* SG_Program -> SG_CgProgram */
	enum {
		SG_VERTEX_PROGRAM,
		SG_FRAGMENT_PROGRAM
	} type;
	CGprogram *_Nullable objs;	/* All compiled programs */
	Uint                nObjs;
	CGprogram instObj;		/* Installed program */
	CGprofile instProf;		/* Installed program profile */
} SG_CgProgram;

__BEGIN_DECLS
extern SG_ProgramClass sgCgProgramClass;
extern CGcontext sgCgProgramCtx;

SG_CgProgram *_Nonnull SG_CgProgramNew(void *_Nonnull, const char *_Nonnull);
__END_DECLS

#endif /* HAVE_CG */
#endif /* _AGAR_SG_CG_PROGRAM_H_ */
