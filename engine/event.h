/*	$Csoft: event.h,v 1.4 2002/05/31 10:48:34 vedge Exp $	*/

typedef union evarg {
	void	*p;
	char	*s;
	char	 c;
	int	 i;
	long int li;
	double	 f;
} *evargs;

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
void	 event_post(void *, char *, const char *fmt, ...);

