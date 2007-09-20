/*	Public domain	*/

#ifndef _AGAR_SG_PROGRAM_H_
#define _AGAR_SG_PROGRAM_H_

#include "begin_code.h"

typedef enum sg_program_type {
	SG_VERTEX_PROGRAM,
	SG_FRAGMENT_PROGRAM
} SG_ProgramType;

typedef struct sg_program {
	struct sg_node node;
	enum sg_program_type type;
	TAILQ_ENTRY(sg_program) progs;
} SG_Program;

__BEGIN_DECLS
extern SG_NodeOps sgProgramOps;

SG_Program	*SG_ProgramNew(void *, const char *, SG_Vector, SG_Real);
SG_Program	*SG_ProgramNewPts(void *, const char *, SG_Vector, SG_Vector,
		                  SG_Vector);
void		 SG_ProgramInit(void *, const char *);
int		 SG_ProgramLoad(void *, AG_Netbuf *);
int		 SG_ProgramSave(void *, AG_Netbuf *);
__END_DECLS

#endif /* _AGAR_SG_PROGRAM_H_ */
