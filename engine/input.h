/*	$Csoft$	*/

/* Input device associated with a map position. */
struct input {
	struct	object obj;
	enum {
		INPUT_KEYBOARD,		/* Keyboard device */
		INPUT_JOY,		/* Joystick device */
		INPUT_MOUSE		/* Mouse device */
	} type;
	int	index;			/* Device index */
	void	*p;			/* User data */

	struct	map_bref *bref;		/* Controlled map position */
};

extern struct input *keyboard, *joy, *mouse;

struct input	*input_create(int, int);
int		 input_destroy(void *);
void		 input_event(void *, SDL_Event *);
int		 input_load(void *, int);
int		 input_save(void *, int);
void	 	 input_dump(void *);

