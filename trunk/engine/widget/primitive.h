/*	$Csoft: primitive.h,v 1.13 2002/11/14 02:17:45 vedge Exp $	*/
/*	Public domain	*/

struct primitive_ops {
	void	(*box)(void *p, int xoffs, int yoffs, int w, int h, int z,
		       Uint32 color);
	void	(*box_rect)(void *p, SDL_Rect *rd, int z, Uint32 color);
	void	(*frame)(void *p, int xoffs, int yoffs, int w, int h,
		         Uint32 color);
	void	(*frame_rect)(void *p, SDL_Rect *rd, Uint32 color);
	void	(*circle)(void *p, int xoffs, int yoffs, int w, int h,
		         int radius, Uint32 color);
	void	(*line)(void *p, int x1, int y1, int x2, int y2, Uint32 color);
	void	(*square)(void *p, int x, int y, int w, int h, Uint32 color);
	void	(*triangle)(void *p, int p1[2], int p2[2], int p3[2],
			    Uint32 color);
};

extern struct primitive_ops primitives;

enum primitive_seq {
	PRIMITIVE_SEQ_MATERIALIZE,
	PRIMITIVE_SEQ_DEMATERIALIZE
};

struct window	*primitive_config_window(void);
void		 primitive_sequence(struct window *, enum primitive_seq);

