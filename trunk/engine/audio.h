/*	$Csoft: audio.h,v 1.1.1.1 2002/01/25 09:50:02 vedge Exp $	*/

struct audio {
	SDL_AudioSpec spec;
	Uint8 *buf;
	Uint32 len;
	int nch;
};


struct audio	*audio_create(char *);

