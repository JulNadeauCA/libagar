/*	$Csoft: input.h,v 1.9 2002/12/13 07:38:15 vedge Exp $	*/
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
	struct object	 obj;
	
	enum input_type	 type;
	int		 index;			/* Device index */
	void		*p;			/* User data */
	
	pthread_mutex_t  lock;
	struct mappos	*pos;			/* Controlled map position */
	TAILQ_ENTRY(input) inputs;
};

struct input	*input_new(int, int);
void		 input_destroy(void *);
void		 input_destroy_all(void);
void		 input_event(enum input_type, SDL_Event *);
struct input	*input_find_ev(enum input_type, SDL_Event *);
struct input	*input_find(char *);

