/*	$Csoft: gfx.h,v 1.3 2003/06/21 06:39:44 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct sample {
	SDL_AudioSpec	 spec;		/* Obtained spec */
	Uint8		*data;		/* Audio data */
	size_t		 size;		/* Size in bytes */
};

struct audio {
	char		  *name;	/* Shared identifier */
	struct sample	  *samples;	/* Audio samples */
	Uint32		  nsamples;
	Uint32		maxsamples;

	pthread_mutex_t	 used_lock;
	Uint32		 used;		/* Reference count */
#define AUDIO_MAX_USED	 (0xffffffff-1)	

	TAILQ_ENTRY(audio) audios;
};

#ifdef DEBUG
#define SAMPLE(ob, i)	audio_get_sample((struct object *)(ob), (i))
#else
#define SAMPLE(ob, i)	((struct object *)(ob))->audio->samples[(i)]
#endif

__BEGIN_DECLS
struct audio	*audio_fetch(void *, const char *);
void		 audio_unused(struct audio *);
void		 audio_wire(struct audio *);
Uint32		 audio_insert_sample(struct audio *, SDL_AudioSpec *, Uint8 *,
		                     size_t);
#ifdef DEBUG
struct object;
__inline__ struct sample *audio_get_sample(struct object *, Uint32);
#endif
__END_DECLS

#include "close_code.h"
