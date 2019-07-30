/*	Public domain	*/

#ifndef _AGAR_AU_WAVE_H_
#define _AGAR_AU_WAVE_H_

#include <agar/config/have_sndfile.h>
#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

#include <agar/au/begin.h>

typedef struct au_wave {
	_Nonnull_Mutex AG_Mutex lock;	/* Lock on audio data */
	Uint flags;
	Uint            nFrames;	/* Number of frames */
	float *_Nullable frames;	/* Uncompressed audio data */
	double peak;			/* Signal peak */
	int ch;				/* Number of channels */
	Uint32 _pad;
#ifdef HAVE_SNDFILE
	SF_INFO info;			/* Format information */
	SNDFILE *_Nullable file;	/* Associated file */
	float   *_Nullable vizFrames;	/* Reduced visualization data */
	sf_count_t        nVizFrames;
#endif
} AU_Wave;

__BEGIN_DECLS
AU_Wave *_Nonnull AU_WaveNew(void)
                            _Warn_Unused_Result;
AU_Wave *_Nullable AU_WaveFromFile(const char *_Nonnull);

void AU_WaveFree(AU_Wave *_Nonnull);
void AU_WaveFreeData(AU_Wave *_Nonnull);
int  AU_WaveLoad(AU_Wave *_Nonnull, const char *_Nonnull);
int  AU_WaveGenVisual(AU_Wave *_Nonnull, int);
__END_DECLS

#include <agar/au/close.h>
#endif /* _AGAR_AU_WAVE_H_ */
