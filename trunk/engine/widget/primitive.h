/*	$Csoft: primitive.h,v 1.9 2002/07/27 07:01:14 vedge Exp $	*/
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

extern const char *primitive_box_sw[];
extern const char *primitive_frame_sw[];
extern const char *primitive_circle_sw[];
extern const char *primitive_line_sw[];
extern const char *primitive_square_sw[];

