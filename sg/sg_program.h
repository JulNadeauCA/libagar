/*	Public domain	*/

#ifndef _AGAR_SG_SG_PROGRAM_H_
#define _AGAR_SG_SG_PROGRAM_H_

typedef struct sg_program_class {
	struct ag_object_class _inherit; /* AG_ObjectClass -> SG_ProgramClass */

	int  (*_Nullable install)(void *_Nonnull, struct sg_view *_Nonnull);
	void (*_Nullable deinstall)(void *_Nonnull, struct sg_view *_Nonnull);
	void (*_Nullable bind)(void *_Nonnull, struct sg_view *_Nonnull);
	void (*_Nullable unbind)(void *_Nonnull, struct sg_view *_Nonnull);
} SG_ProgramClass;

typedef struct sg_program {
	struct ag_object obj;		/* AG_Object -> SG_Program */
	Uint flags;
	int tag;			/* User tag */
} SG_Program;

#define SG_PROGRAM(sp)	   ((SG_Program *)(sp))
#define SG_PROGRAM_OPS(sp) ((SG_ProgramClass *)AGOBJECT(sp)->cls)

__BEGIN_DECLS
extern AG_ObjectClass sgProgramClass;

int  SG_ProgramInstall(SG_Program *_Nonnull, struct sg_view *_Nonnull);
void SG_ProgramDeinstall(SG_Program *_Nonnull, struct sg_view *_Nonnull);
void SG_ProgramBind(SG_Program *_Nonnull, struct sg_view *_Nonnull);
void SG_ProgramUnbind(SG_Program *_Nonnull, struct sg_view *_Nonnull);
__END_DECLS

#endif /* _AGAR_SG_SG_PROGRAM_H_ */
