/*	Public domain	*/

#ifndef _AGAR_SG_PE_H_
#define _AGAR_SG_PE_H_

typedef struct pe_ops {
	struct ag_object_ops ops;
	int  (*attachObject)(void *, SG_Object *);
	void (*detachObject)(void *, SG_Object *);
} PE_Ops;

typedef struct pe {
	struct ag_object obj;
	Uint flags;
	SG_Real time;			/* Simulation clock */
} PE;

#define PE_OPS(pe) ((PE_Ops *)AGOBJECT(pe)->ops)

__BEGIN_DECLS
extern const AG_ObjectOps peOps;

void	 PE_Init(void *, const char *);
void	 PE_Destroy(void *);
int	 PE_Load(void *, AG_Netbuf *);
int	 PE_Save(void *, AG_Netbuf *);

void	 PE_AttachObject(PE *, SG_Object *);
void	 PE_DetachObject(PE *, SG_Object *);
__END_DECLS

#endif /* _AGAR_SG_PE_H_ */
