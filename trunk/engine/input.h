/*	$Csoft: input.h,v 1.5 2002/05/02 06:25:09 vedge Exp $	*/

struct object;
struct mappos;

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
	struct	mappos *pos;		/* Controlled map position */
	pthread_mutex_t lock;		/* Lock on the whole structure */
};

extern struct input *keyboard, *joy, *mouse;

struct input	*input_new(int, int);
void		 input_destroy(void *);
void		 input_event(void *, SDL_Event *);

