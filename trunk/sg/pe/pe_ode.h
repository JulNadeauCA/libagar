/*	Public domain	*/

#ifndef _AGAR_SG_PE_ODE_H_
#define _AGAR_SG_PE_ODE_H_

#ifdef _AGAR_INTERNAL
#include <config/have_ode.h>
#else
#include <agar/config/have_ode.h>
#endif

#ifdef HAVE_ODE

#include <ode/ode.h>

typedef struct pe_ode_node_data {
	dBodyID body;
	dGeomID geom;
} PE_OdeNodeData;

typedef struct pe_ode {
	struct pe pe;
} PE_Ode;

__BEGIN_DECLS
extern const PE_Class peOdeClass;

PE_Ode *PE_OdeNew(void *, const char *);
__END_DECLS

#endif /* HAVE_CG */
#endif /* _AGAR_SG_PE_ODE_H_ */
