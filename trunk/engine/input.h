/*	$Csoft: input.h,v 1.7 2002/06/09 10:08:04 vedge Exp $	*/
/*	Public domain	*/

struct object;
struct mappos;

enum input_type {
	INPUT_KEYBOARD,		/* Keyboard device */
	INPUT_JOY,		/* Joystick device */
	INPUT_MOUSE		/* Mouse device */
};

/* Input device associated with a map position. */
struct input {
	struct	object obj;
	enum	input_type type;
	int	index;			/* Device index */
	void	*p;			/* User data */
	struct	mappos *pos;		/* Controlled map position */
	TAILQ_ENTRY(input) inputs;
	pthread_mutex_t lock;		/* Lock on the whole structure */
};

struct input	*input_new(int, int);
void		 input_destroy(void *);
void		 input_destroy_all(void);
void		 input_event(enum input_type, SDL_Event *);
struct input	*input_find_ev(enum input_type, SDL_Event *);
struct input	*input_find_str(char *);

