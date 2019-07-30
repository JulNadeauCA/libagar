/*	Public domain	*/

#include <agar/core/begin.h>

#ifndef AU_MINBUFSIZ
#define AU_MINBUFSIZ 65536
#endif

struct au_dev_out;

typedef struct au_dev_out_class {
	const char *_Nonnull name;		/* Display text */
#ifdef AG_HAVE_64BIT
	Uint64 size;				/* Instance structure size */
#else
	Uint size;				/* Instance structure size */
#endif
	void (*_Nullable Init)(void *_Nonnull);
	void (*_Nullable Destroy)(void *_Nonnull);
	int  (*_Nullable Open)(void *_Nonnull, const char *_Nonnull, int, int);
	void (*_Nullable Close)(void *_Nonnull);
} AU_DevOutClass;

#if 0
/* TODO */
/* Buffered audio connection */
typedef struct au_link {
	enum au_link_type {
		AU_LINK_SIGNAL,		/* Buffered audio */
		AU_LINK_MIDI,		/* Timestamped MIDI events */
	} type;
	Uint bytesPerFrame;		/* Bytes per frame */
	struct au_dev_out *outDev;	/* Output device */
	Uint               outCh;	/* Output virtual channel */
	_Nullable_Mutex AG_Mutex lock;	/* Lock protecting data */
	union {
		struct {
			Uint rate;	/* Sampling rate (Hz) */
			Uint ch;	/* Channel count */
			float *buf;	/* Audio buffer */
			AG_Size size;
		} signal;
		struct {
			Uint8 *buf;	/* MIDI buffer */
			AG_Size size;
		} midi;
	} data;
	AG_TAILQ_ENTRY(au_link) src;	/* Links in source */
	AG_TAILQ_ENTRY(au_link) chan;	/* Links in DevOut virtual channels */
} AU_Link;
#endif

typedef struct au_channel {
	float vol;			/* Channel volume */
	float pan;			/* Stereo panning */
#if 0
	AG_TAILQ_HEAD_(au_link) links;	/* Device connections */
#endif
} AU_Channel;

typedef struct au_dev_out {
#ifdef AG_THREADS
	_Nonnull_Mutex AG_Mutex lock;		/* Lock protecting access */
#endif
	const AU_DevOutClass *_Nonnull cls;	/* Class description */
	Uint flags;
#define AU_DEV_OUT_THREADED	0x01	/* Device uses separate threads */
#define AU_DEV_OUT_CLOSING	0x02	/* Device is being shut down */
#define AU_DEV_OUT_ERROR	0x04	/* I/O error has occurred */

	int rate;			/* Sample rate */
	int ch;				/* Channel count */
	int bytesPerFrame;		/* Bytes per audio frame */
	float *_Nonnull buf;		/* Audio buffer */
	AG_Size bufSize;		/* Buffer content size (frames) */
	AG_Size bufMax;			/* Total buffer size (frames) */
	int nOverruns;			/* Overruns recorded */
	Uint                 nChan;
	AU_Channel *_Nullable chan;	/* Virtual channels */
#ifdef AG_THREADS
	_Nonnull AG_Cond wrRdy, rdRdy;	/* Buffer status */
#endif
} AU_DevOut;

#define AUDEVOUT(obj) ((AU_DevOut *)(obj))

__BEGIN_DECLS
extern const AU_DevOutClass *_Nullable auDevOutList[];

AU_DevOut *_Nullable AU_OpenOut(const char *_Nonnull, int, int);
void                 AU_CloseOut(AU_DevOut *_Nonnull);

int AU_WriteFloat(AU_DevOut *_Nonnull, float *_Nonnull, Uint);

int AU_AddChannel(AU_DevOut *_Nonnull);
int AU_DelChannel(AU_DevOut *_Nonnull, int);
__END_DECLS

#include <agar/core/close.h>
