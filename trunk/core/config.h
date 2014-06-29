/*	Public domain	*/

#ifndef _AGAR_CORE_CONFIG_H_
#define _AGAR_CORE_CONFIG_H_
#include <agar/core/begin.h>

typedef struct ag_object AG_Config;

__BEGIN_DECLS
extern AG_Config *agConfig;
extern AG_ObjectClass agConfigClass;

AG_Config *AG_ConfigObject(void);
int  AG_ConfigInit(AG_Config *, Uint);
int  AG_ConfigFile(const char *, const char *, const char *, char *, size_t)
		   BOUNDED_ATTRIBUTE(__string__, 4, 5);
int  AG_CreateDataDir(void);
int  AG_ConfigSave(void);
int  AG_ConfigLoad(void);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_CONFIG_H_ */
