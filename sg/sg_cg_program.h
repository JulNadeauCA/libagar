/*	Public domain	*/

#ifndef _AGAR_SG_CG_PROGRAM_H_
#define _AGAR_SG_CG_PROGRAM_H_

#ifdef _AGAR_INTERNAL
#include <config/have_cg.h>
#else
#include <agar/config/have_cg.h>
#endif

#ifdef HAVE_CG

#include <Cg/cg.h>
#include <Cg/cgGL.h>

typedef struct sg_cg_program {
	struct sg_program sp;
	enum {
		SG_VERTEX_PROGRAM,
		SG_FRAGMENT_PROGRAM
	} type;
	CGprogram *objs;		/* All compiled programs */
	Uint nObjs;
	CGprogram instObj;		/* Installed program */
	CGprofile instProf;		/* Installed program profile */
} SG_CgProgram;

__BEGIN_DECLS
extern const SG_ProgramClass sgCgProgramClass;
extern CGcontext sgCgProgramCtx;

SG_CgProgram *SG_CgProgramNew(void *, const char *);
__END_DECLS

#endif /* HAVE_CG */
#endif /* _AGAR_SG_CG_PROGRAM_H_ */
