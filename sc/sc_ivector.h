/*	$Csoft: veci.h,v 1.2 2005/09/10 05:06:06 vedge Exp $	*/
/*	Public domain	*/

struct veci {
	int n;			/* Length */
	int *vec;		/* Vector data */
};

typedef struct veci SC_Ivector;

__BEGIN_DECLS
SC_Ivector	*SC_IvectorNew(Uint);
__inline__ void	 SC_IvectorSet(SC_Ivector *, int);
#define		 SC_IvectorSetZero(iv) SC_IvectorSet((iv),0)
__inline__ void	 SC_IvectorCopy(const SC_Ivector *, SC_Ivector *);
__inline__ void	 SC_IvectorFree(SC_Ivector *);
__inline__ void	 SC_IvectorAdd(const SC_Ivector *, SC_Ivector *);
__inline__ void	 SC_IvectorResize(SC_Ivector *, Uint);
#ifdef DEBUG
void		 SC_IvectorPrint(const SC_Ivector *);
#endif
__END_DECLS
