/*	$Csoft: event.h,v 1.3 2002/05/02 06:29:41 vedge Exp $	*/

union evarg {
	void	*p;
	char	*s;
	char	 c;
	int	 i;
	long int li;
	double	 f;
};

#define EVENT_MAXARGS	16

struct event {
	char	*name;
	int	 flags;
#define	EVENT_ASYNC	0x01	/* Event handler runs in own thread */
	union	 evarg argv[EVENT_MAXARGS];
	int	 argc;
	void	(*handler)(int, union evarg *);
	TAILQ_ENTRY(event) events;
};

/* SDL user events */
enum {
	USER_WINDOW_EVENT
};

void	*event_loop(void *);
void	 event_new(void *, char *, int, void (*)(int, union evarg *),
	     const char *, ...);
void	 event_post(void *, const char *, const char *fmt, ...);

