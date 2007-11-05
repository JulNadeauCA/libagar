/*	Public domain	*/

#include "begin_code.h"

typedef struct ag_object AG_Config;
extern AG_Config *agConfig;

__BEGIN_DECLS
extern const AG_ObjectOps agConfigOps;

extern int agKbdUnicode, agKbdDelay, agKbdRepeat;
extern int agMouseDblclickDelay, agMouseSpinDelay, agMouseSpinIval;
extern int agTextComposition, agTextBidi, agTextAntialiasing, agTextTabWidth,
	   agTextBlinkRate, agTextAntialiasing, agPageIncrement, agTextSymbols;
extern int agIdleThresh;
extern int agScreenshotQuality;
extern int agWindowAnySize;
extern int agMsgDelay;

void AG_ConfigInit(AG_Config *);
int  AG_ConfigFile(const char *, const char *, const char *, char *, size_t)
		   BOUNDED_ATTRIBUTE(__string__, 4, 5);
__END_DECLS

#include "close_code.h"
