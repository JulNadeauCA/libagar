/*	$Csoft: primitive.h,v 1.11 2002/09/07 04:36:59 vedge Exp $	*/
/*	Public domain	*/

struct primitive_ops {
	void	(*box)(void *p, int xoffs, int yoffs, int w, int h, int z,
		       Uint32 color);
	void	(*frame)(void *p, int xoffs, int yoffs, int w, int h,
		         Uint32 color);
	void	(*circle)(void *p, int xoffs, int yoffs, int w, int h,
		         int radius, Uint32 color);
	void	(*line)(void *p, int x1, int y1, int x2, int y2, Uint32 color);
	void	(*square)(void *p, int x, int y, int w, int h, Uint32 color);
	void	(*triangle)(void *p, int p1[2], int p2[2], int p3[2],
			    Uint32 color);
};

extern struct primitive_ops primitives;

struct window	*primitive_config_window(void);

