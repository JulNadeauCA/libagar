/*	$Csoft: primitive.h,v 1.15 2002/12/26 07:03:22 vedge Exp $	*/
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
	void	(*rect_outlined)(void *p, int x, int y, int w, int h,
		                 Uint32 color);
	void	(*rect_filled)(void *p, SDL_Rect *rd, Uint32 color);
};

extern struct primitive_ops primitives;

struct window	*primitive_config_window(void);
void		 primitives_init(void);

