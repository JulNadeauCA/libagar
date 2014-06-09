/*	Public domain	*/

#include <agar/core/begin.h>

#define AU_MINBUFSIZ	65536

struct au_dev_out;

typedef struct au_dev_out_class {
	const char *name;
	size_t size;
	void (*Init)(void *);
	void (*Destroy)(void *);
	int  (*Open)(void *, const char *path, int rate, int channels);
	void (*Close)(void *);
} AU_DevOutClass;

/* Buffered audio connection */
typedef struct au_link {
	enum au_link_type {
		AU_LINK_SIGNAL,		/* Buffered audio */
		AU_LINK_MIDI,		/* Timestamped MIDI events */
	} type;
	Uint bytesPerFrame;		/* Bytes per frame */
	struct au_dev_out *outDev;	/* Output device */
	Uint       outCh;		/* Output virtual channel */
	AG_Mutex lock;
	union {
		struct {
			Uint rate;	/* Sampling rate (Hz) */
			Uint ch;	/* Channel count */
			float *buf;	/* Audio buffer */
			size_t size;
		} signal;
		struct {
			Uint8 *buf;	/* MIDI buffer */
			size_t size;
		} midi;
	} data;
	AG_TAILQ_ENTRY(au_link) src;	/* Links in source */
	AG_TAILQ_ENTRY(au_link) chan;	/* Links in DevOut virtual channels */
} AU_Link;

typedef struct au_channel {
	float vol;			/* Channel volume */
	float pan;			/* Stereo panning */
	AG_TAILQ_HEAD_(au_link) links;	/* Device connections */
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
	size_t bufSize;		/* Buffer content size (frames) */
	size_t bufMax;		/* Total buffer size (frames) */
	int nOverruns;		/* Overruns occured */
	AG_Cond wrRdy, rdRdy;	/* Buffer status */
	AU_Channel *chan;	/* Virtual channels */
	Uint       nChan;
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
	AG_MutexLock(&dev->lock);
	if (dev->bufSize+frames > dev->bufMax) {
		float *bufNew;
		if ((bufNew = AG_TryRealloc(dev->buf,
		    (dev->bufSize+frames)*dev->bytesPerFrame)) == NULL) {
			AG_MutexUnlock(&dev->lock);
			return (-1);
		}
		dev->buf = bufNew;
		dev->bufMax = dev->bufSize+frames;
	}
	memcpy(&dev->buf[dev->bufSize*dev->ch], data, frames*dev->bytesPerFrame);
	dev->bufSize += frames;
	AG_CondBroadcast(&dev->rdRdy);
	AG_MutexUnlock(&dev->lock);
	return (0);
}
__END_DECLS

#include <agar/core/close.h>
