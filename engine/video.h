/*	$Csoft: video.h,v 1.3 2002/01/28 05:09:01 vedge Exp $	*/

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

struct video	*video_create(char *, SDL_Surface *);
void		 video_destroy(struct video *);

