/*	$Csoft: input.h,v 1.11 2003/04/12 01:45:31 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_INPUT_H_
#define _AGAR_INPUT_H
#include "begin_code.h"

struct object;
struct object_position;

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
	
	pthread_mutex_t		 lock;
	struct object_position	*pos;		/* Controlled map position */

	TAILQ_ENTRY(input) inputs;
};

__BEGIN_DECLS
extern DECLSPEC struct input	*input_new(int, int);
extern DECLSPEC void		 input_destroy(void *);
extern DECLSPEC void		 input_destroy_all(void);
extern DECLSPEC void		 input_event(enum input_type, SDL_Event *);
extern DECLSPEC struct input	*input_find_ev(enum input_type, SDL_Event *);
extern DECLSPEC struct input	*input_find(char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_INPUT_H_ */
