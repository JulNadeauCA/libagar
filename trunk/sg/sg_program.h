/*	Public domain	*/

#ifndef _AGAR_SG_PROGRAM_H_
#define _AGAR_SG_PROGRAM_H_

#include "begin_code.h"

typedef enum sg_program_type {
	SG_VERTEX_PROGRAM,
	SG_FRAGMENT_PROGRAM
} SG_ProgramType;

typedef struct sg_program {
	enum sg_program_type type;
	TAILQ_ENTRY(sg_program) progs;
} SG_Program;

#endif /* _AGAR_SG_PROGRAM_H_ */
