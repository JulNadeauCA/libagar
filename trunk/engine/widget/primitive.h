/*	$Csoft: primitive.h,v 1.10 2002/08/18 00:42:57 vedge Exp $	*/
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
};

extern struct primitive_ops primitives;

extern char *primitive_box_sw[];
extern char *primitive_frame_sw[];
extern char *primitive_circle_sw[];
extern char *primitive_line_sw[];
extern char *primitive_square_sw[];

