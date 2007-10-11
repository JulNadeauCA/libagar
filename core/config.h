/*	Public domain	*/

#include "begin_code.h"

typedef struct ag_object AG_Config;
extern AG_Config *agConfig;

__BEGIN_DECLS
extern int agKbdUnicode;
extern int agKbdDelay;
extern int agKbdRepeat;
extern int agMouseDblclickDelay;
extern int agMouseSpinDelay;
extern int agMouseSpinIval;

extern int agTextAntialiasing;
extern int agTextComposition;
extern int agTextBidi;
extern int agTextTabWidth;
extern int agIdleThresh;
extern int agScreenshotQuality;
extern int agWindowAnySize;

void AG_ConfigInit(AG_Config *);
int  AG_ConfigLoad(void *, AG_Netbuf *);
int  AG_ConfigSave(void *, AG_Netbuf *);
int  AG_ConfigFile(const char *, const char *, const char *, char *, size_t)
		   BOUNDED_ATTRIBUTE(__string__, 4, 5);
__END_DECLS

#include "close_code.h"
