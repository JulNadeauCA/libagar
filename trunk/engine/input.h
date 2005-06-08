/*	$Csoft: input.h,v 1.18 2004/02/20 04:20:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_INPUT_H_
#define _AGAR_INPUT_H

#include "begin_code.h"

#define AG_INPUT_NAME_MAX	16

enum ag_input_type {
	AG_INPUT_KEYBOARD,		/* Keyboard device */
	AG_INPUT_JOY,			/* Joystick device */
	AG_INPUT_MOUSE			/* Mouse device */
};

/* Driver for a specific type of input device. */
typedef struct ag_input_ops {
	char	*name;
	void	(*in_close)(void *);
	int	(*in_match)(const void *, const SDL_Event *);
	void	(*in_event)(void *, const SDL_Event *);
} AG_InputOps;

SLIST_HEAD(ag_input_devq, ag_input);

/* Input device mapping. */
typedef struct ag_input {
	char name[AG_INPUT_NAME_MAX];		/* Device identifier */
	enum ag_input_type type;		/* Type of device */
	const AG_InputOps *ops;			/* Generic operations */
	SLIST_ENTRY(ag_input) inputs;
} AG_Input;

__BEGIN_DECLS
void	 AG_InputSet(void *, enum ag_input_type, const char *,
	             const AG_InputOps *);
void	 AG_InputRemove(void *);
void	 AG_InputDestroy(void);
void	 AG_InputEvent(enum ag_input_type, const SDL_Event *);
void	*AG_InputFind(const char *);
__END_DECLS

#include "close_code.h"

#include <engine/input/kbd.h>
#include <engine/input/joy.h>
#include <engine/input/mouse.h>

#endif	/* _AGAR_INPUT_H_ */
