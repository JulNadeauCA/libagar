/*	$Csoft: input.h,v 1.15 2003/06/18 00:46:58 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_INPUT_H_
#define _AGAR_INPUT_H

#include "begin_code.h"

#define INPUT_NAME_MAX	16

struct object_position;

enum input_type {
	INPUT_KEYBOARD,		/* Keyboard device */
	INPUT_JOY,		/* Joystick device */
	INPUT_MOUSE		/* Mouse device */
};

/* Driver for a specific type of input device. */
struct input_driver {
	char	*name;						/* Identifier */
	void	(*in_close)(void *);				/* Clean up */
	int	(*in_match)(const void *, const SDL_Event *);	/* Map event */
	void	(*in_event)(void *, const SDL_Event *);		/* Proc event */
};

/* Input device associated with a map position. */
struct input {
	char		name[INPUT_NAME_MAX];	/* Device identifier */
	enum input_type	type;			/* Type of device */

	const struct input_driver *drv;		/* Input driver */
	struct object_position	  *pos;		/* Position to control */

	SLIST_ENTRY(input) inputs;
};

__BEGIN_DECLS
void	 input_register(void *, enum input_type, const char *,
	                const struct input_driver *);
void	 input_deregister(void *);
void	 input_destroy(void);
void	 input_event(enum input_type, const SDL_Event *);
void	*input_find(const char *);
__END_DECLS

#include "close_code.h"

#include <engine/input/kbd.h>
#include <engine/input/joy.h>
#include <engine/input/mouse.h>

#endif	/* _AGAR_INPUT_H_ */
