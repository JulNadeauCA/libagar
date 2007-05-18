/*	$Csoft: lu.h,v 1.2 2005/09/10 05:06:06 vedge Exp $	*/
/*	Public domain	*/

__BEGIN_DECLS
SC_Matrix *SC_FactorizeLU(const SC_Matrix *, SC_Matrix *, SC_Ivector *,
                          SC_Real *);
void	   SC_BacksubstLU(const SC_Matrix *, const SC_Ivector *, SC_Vector *);
__END_DECLS
