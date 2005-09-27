/*	$Csoft: audio.h,v 1.10 2005/07/16 16:07:27 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

typedef struct ag_audio_sample {
	SDL_AudioSpec spec;		/* Obtained spec */
	Uint8 *data;			/* Audio data */
	size_t size;			/* Size in bytes */
} AG_AudioSample;

typedef struct ag_audio {
	char *name;				/* Shared identifier */
	struct ag_audio_sample *samples;	/* Audio samples */
	Uint32 nsamples;
	Uint32 maxsamples;
	Uint32 used;				/* Reference count */
#define AG_AUDIO_MAX_USED (0xffffffff-1)	
	TAILQ_ENTRY(ag_audio) audios;
} AG_Audio;

__BEGIN_DECLS
AG_Audio *AG_AudioFetch(const char *);
void	  AG_AudioDestroy(AG_Audio *);
void	  AG_AudioWire(AG_Audio *);
Uint32	  AG_AudioInsertSample(AG_Audio *, SDL_AudioSpec *, Uint8 *, size_t);
__END_DECLS

#include "close_code.h"
