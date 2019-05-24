/*	Public domain	*/

#ifndef _AGAR_CORE_CONFIG_H_
#define _AGAR_CORE_CONFIG_H_

#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <agar/core/begin.h>

typedef enum ag_config_path_group {
	AG_CONFIG_PATH_DATA,	/* Objects and data from */
	AG_CONFIG_PATH_FONTS,	/* Font files */
	AG_CONFIG_PATH_LAST
} AG_ConfigPathGroup;

typedef struct ag_config_path {
	char *_Nonnull s;			/* Path to directory */
	AG_SLIST_ENTRY(ag_config_path) paths;
} AG_ConfigPath;

typedef AG_SLIST_HEAD(ag_config_pathq, ag_config_path) AG_ConfigPathQ;

typedef struct ag_config {
	struct ag_object _inherit;              /* AG_Object(3) -> AG_Config */
	AG_ConfigPathQ paths[AG_CONFIG_PATH_LAST];  /* Configured load paths */
} AG_Config;

__BEGIN_DECLS
extern AG_Config *_Nullable agConfig;
extern AG_ObjectClass agConfigClass;

AG_Config *_Nullable AG_ConfigObject(void);

int  AG_ConfigFind(AG_ConfigPathGroup, const char *_Nonnull,
                   char *_Nonnull, AG_Size);
void AG_ConfigAddPathS(AG_ConfigPathGroup, const char *_Nonnull);
void AG_ConfigAddPath(AG_ConfigPathGroup, const char *_Nonnull, ...)
                     FORMAT_ATTRIBUTE(printf,2,3);
void AG_ConfigDelPathS(AG_ConfigPathGroup, const char *_Nonnull);
void AG_ConfigDelPath(AG_ConfigPathGroup, const char *_Nonnull, ...)
                     FORMAT_ATTRIBUTE(printf,2,3);
void AG_ConfigClearPaths(AG_Config *_Nonnull);

int AG_ConfigInit(AG_Config *_Nonnull, Uint);

int AG_CreateDataDir(void);
int AG_ConfigSave(void);
int AG_ConfigLoad(void);

#ifdef AG_LEGACY
int  AG_ConfigFile(const char *_Nonnull, const char *_Nonnull,
                  const char *_Nullable, char *_Nonnull, AG_Size)
		  DEPRECATED_ATTRIBUTE;
/* -> AG_ConfigFind() */
#endif
__END_DECLS

#include <agar/core/close.h>
#endif /* AG_SERIALIZATION */
#endif /* _AGAR_CORE_CONFIG_H_ */
