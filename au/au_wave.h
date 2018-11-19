/*	Public domain	*/

#ifndef _AGAR_AU_WAVE_H_
#define _AGAR_AU_WAVE_H_

#include <sndfile.h>

#include <agar/au/begin.h>

typedef struct au_wave {
	_Nonnull AG_Mutex lock;		/* Lock on audio data */
	Uint flags;
	SNDFILE *_Nullable file;	/* Associated file */
	SF_INFO info;			/* Format information */
	float *_Nullable frames;	/* Uncompressed audio data */
	Uint            nFrames;	/* Number of frames */
	int ch;				/* Number of channels */
	float *_Nullable vizFrames;	/* Reduced visualization data */
	sf_count_t nVizFrames;		/* Visualization # frames total */
	double peak;			/* Signal peak */
} AU_Wave;

typedef struct au_wave_state {
	int play;			/* Sample is playing */
	sf_count_t pos;			/* Playback position */
} AU_WaveState;

__BEGIN_DECLS
AU_Wave *_Nonnull AU_WaveNew(void) _Warn_Unused_Result;
AU_Wave *_Nullable AU_WaveFromFile(const char *_Nonnull);

void AU_WaveFree(AU_Wave *_Nonnull);
void AU_WaveFreeData(AU_Wave *_Nonnull);
int  AU_WaveLoad(AU_Wave *_Nonnull, const char *_Nonnull);
int  AU_WaveGenVisual(AU_Wave *_Nonnull, int);
__END_DECLS

#include <agar/au/close.h>
#endif /* _AGAR_AU_WAVE_H_ */
