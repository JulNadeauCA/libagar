/*	$Csoft: primitive.h,v 1.7 2002/07/21 10:56:44 vedge Exp $	*/
/*	Public domain	*/

struct primitive_ops {
	void	(*box)(void *p, int xoffs, int yoffs, int w, int h, int z);
	void	(*frame)(void *p, int xoffs, int yoffs, int w, int h,
		         Uint32 color);
	void	(*circle)(void *p, int xoffs, int yoffs, int w, int h,
		         int radius, Uint32 color);
	void	(*line)(void *p, int x1, int y1, int x2, int y2, Uint32 color);
	void	(*square)(void *p, int x, int y, int w, int h, Uint32 color);
};

extern struct primitive_ops primitives;

