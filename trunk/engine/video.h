/*	$Csoft: video.h,v 1.5 2002/04/24 13:18:38 vedge Exp $	*/

#ifdef CONF_SMPEG
#include <smpeg.h>
#endif

struct video {
	struct	object obj;

	pthread_t	thread;

	/* Private */
#ifdef CONF_SMPEG
	int		fd;
	SMPEG	  	*mpeg;
	SDL_mutex	*lock;
#endif
};

void	video_init(struct video *, char *, SDL_Surface *);
void	video_destroy(void *);

