/*	Public domain	*/

#include <agar/core/begin.h>

#define AU_BUFSIZ 524288

typedef struct au_dev_out_class {
	const char *name;
	size_t size;
	void (*Init)(void *);
	void (*Destroy)(void *);
	int  (*Open)(void *, const char *path, int rate, int channels);
	void (*Close)(void *);
} AU_DevOutClass;

typedef struct au_channel {
	float vol;			/* Channel volume */
	float pan;			/* Stereo panning */
} AU_Channel;

typedef struct au_dev_out {
	AG_Mutex lock;
	const AU_DevOutClass *cls;
	Uint flags;
#define AU_DEV_OUT_THREADED	0x01	/* Device uses separate threads */
#define AU_DEV_OUT_CLOSING	0x02	/* Device is being shut down */
#define AU_DEV_OUT_ERROR	0x04	/* I/O error occured */

	int rate;		/* Sample rate */
	int ch;			/* Channel count */
	int bytesPerFrame;	/* Bytes per audio frame */
	float *buf;		/* Audio buffer */
	size_t bufSize;		/* Current buffer size (frames) */
	int nOverruns;		/* Overruns occured */
	AG_Cond wrRdy, rdRdy;	/* Buffer status */
	AU_Channel *mix;	/* Mixing channels */
	int        nMix;
} AU_DevOut;

#define AUDEVOUT(obj) ((AU_DevOut *)(obj))

__BEGIN_DECLS
extern const AU_DevOutClass *auDevOutList[];

AU_DevOut *AU_OpenOut(const char *, int, int);
void       AU_CloseOut(AU_DevOut *);

int        AU_AddChannel(AU_DevOut *);
int        AU_DelChannel(AU_DevOut *, int);

static __inline__ int
AU_WriteFloat(AU_DevOut *dev, float *data, size_t frames)
{
	if (dev->bufSize+frames >= AU_BUFSIZ) {
		return (-1);
	}
	AG_MutexLock(&dev->lock);
	memcpy(&dev->buf[dev->bufSize*dev->ch], data, frames*dev->bytesPerFrame);
	dev->bufSize += frames;
	AG_CondBroadcast(&dev->rdRdy);
	AG_MutexUnlock(&dev->lock);
	return (0);
}
__END_DECLS

#include <agar/core/close.h>
