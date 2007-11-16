/*	Public domain	*/

#ifndef _AGAR_SG_PROGRAM_H_
#define _AGAR_SG_PROGRAM_H_

typedef struct sg_program_class {
	struct ag_object_class inherit;
	int  (*install)(void *, SG_View *);
	void (*deinstall)(void *, SG_View *);
	void (*bind)(void *, SG_View *);
	void (*unbind)(void *, SG_View *);
} SG_ProgramClass;

typedef struct sg_program {
	struct ag_object obj;
	Uint flags;
} SG_Program;

#define SG_PROGRAM(sp)		((SG_Program *)(sp))
#define SG_PROGRAM_OPS(sp)	((SG_ProgramClass *)AGOBJECT(sp)->cls)

__BEGIN_DECLS
extern const AG_ObjectClass sgProgramClass;

void	 SG_ProgramInstall(SG_Program *, SG_View *);
void	 SG_ProgramDeinstall(SG_Program *, SG_View *);
void	 SG_ProgramBind(SG_Program *, SG_View *);
void	 SG_ProgramUnbind(SG_Program *, SG_View *);
__END_DECLS

#endif /* _AGAR_SG_PROGRAM_H_ */
