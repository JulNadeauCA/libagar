/*	$Csoft: primitive.h,v 1.5 2002/07/20 16:15:29 vedge Exp $	*/
/*	Public domain	*/

struct primitive_ops {
	void	(*box)(void *, int, int, int, int, int);
	void	(*frame)(void *, int, int, int, int, int);
	void	(*circle)(void *, int, int, int, int, int, int);
	void	(*line)(void *, int, int, int, int, Uint32);
};

extern struct primitive_ops primitives;

