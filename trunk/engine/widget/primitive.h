/*	$Csoft: primitive.h,v 1.4 2002/07/07 00:24:44 vedge Exp $	*/
/*	Public domain	*/

struct primitive_ops {
	void	(*box)(void *, int, int, int, int, int);
	void	(*frame)(void *, int, int, int);
	void	(*circle)(void *, int, int, int, int);
};

extern struct primitive_ops primitives;

