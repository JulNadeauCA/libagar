/*	$Csoft: video.h,v 1.2 2002/01/26 06:37:32 vedge Exp $	*/

#include <smpeg.h>

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

