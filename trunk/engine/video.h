/*	$Csoft: video.h,v 1.1 2002/01/26 03:37:38 vedge Exp $	*/

#include <smpeg.h>

struct video {
	int	flags;
#define VIDEO_STREAM	0x01
	int	fd;

	SMPEG	  	*mpeg;
	pthread_t	thread;	/* Playback thread */
	SDL_mutex	*lock;	/* Lock on mpeg */
};

struct video	*video_create(char *, SDL_Surface *);
void		 video_destroy(struct video *);

