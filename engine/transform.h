/*	$Csoft$	*/
/*	Public domain	*/

enum transform_type {
	TRANSFORM_SCALE,
	TRANSFORM_HFLIP,
	TRANSFORM_VFLIP,
	TRANSFORM_ROTATE,
	TRANSFORM_COLOR,
	TRANSFORM_PIXELIZE,
	TRANSFORM_RANDOMIZE
};

struct transform {
	enum transform_type  type;

	SDL_Surface	*(*func)(SDL_Surface *, struct transform *);
	SDL_Surface	*cached;

	union {
		struct {
			Uint16	w, h;
		} scale;
		struct {
			Uint16	angle;
		} rotate;
		struct {
			Uint8	r, g, b, a;
		} color;
		struct {
			Uint16	factor;
		} pixelize;
		struct {
			Uint8	nrounds;
			Uint8	r_range, g_range, b_range, a_range;
		} randomize;
	} args;

	SLIST_ENTRY(transform)	 transforms;
};

void	transform_init(struct transform *, enum transform_type);
void	transform_load(int, struct transform *);
void	transform_save(void *, struct transform *);
void	transform_destroy(struct transform *);
void	transform_copy(struct transform *, struct transform *);

