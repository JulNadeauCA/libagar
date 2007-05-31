/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _SC_GUI_H_
#define _SC_GUI_H_
#include <agar/gui/gui.h>

#if SC_PRECISION == SC_LONG_DOUBLE
# define SC_WidgetBindReal(w,n,p) AG_WidgetBind((w),(n),AG_WIDGET_LONG_DOUBLE,\
                                                (p))
#elif SC_PRECISION == SC_DOUBLE
# define SC_WidgetBindReal(w,n,p) AG_WidgetBind((w),(n),AG_WIDGET_DOUBLE,(p))
#else
# define SC_WidgetBindReal(w,n,p) AG_WidgetBind((w),(n),AG_WIDGET_FLOAT,(p))
#endif

__BEGIN_DECLS
AG_FSpinbutton	*SC_SpinReal(void *, const char *, SC_Real *, SC_Real);
void		*SC_EditVector3(void *, const char *, SC_Vector *, SC_Real);
__END_DECLS
#endif /* _SC_GUI_H_ */
