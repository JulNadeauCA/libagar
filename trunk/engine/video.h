/*	$Csoft: video.h,v 1.4 2002/01/28 05:23:15 vedge Exp $	*/

#ifdef CONF_SMPEG
#include <smpeg.h>
#endif

struct video {
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

