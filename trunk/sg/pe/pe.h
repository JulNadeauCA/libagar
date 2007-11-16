/*	Public domain	*/

#ifndef _AGAR_SG_PE_H_
#define _AGAR_SG_PE_H_

typedef struct pe_class {
	struct ag_object_class _inherit;
	int  (*attachObject)(void *, SG_Object *);
	void (*detachObject)(void *, SG_Object *);
} PE_Class;

typedef struct pe {
	struct ag_object obj;
	Uint flags;
	SG_Real time;			/* Simulation clock */
} PE;

#define PE_OPS(pe) ((PE_Class *)AGOBJECT(pe)->cls)

__BEGIN_DECLS
extern const AG_ObjectClass peClass;

void	 PE_AttachObject(PE *, SG_Object *);
void	 PE_DetachObject(PE *, SG_Object *);
__END_DECLS

#endif /* _AGAR_SG_PE_H_ */
