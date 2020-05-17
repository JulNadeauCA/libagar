/*	Public domain	*/

#include <agar/core/begin.h>

#ifndef AG_USER_NAME_MAX
#define AG_USER_NAME_MAX 68
#endif

/* User account (POSIX-style, or otherwise) */
typedef struct ag_user {
	char	 name[AG_USER_NAME_MAX];	/* User name */
	Uint     flags;
#define AG_USER_NO_ACCOUNT	0x01		/* Not a real user account */
	char    *_Nullable passwd;		/* Encrypted password */
	Uint32   uid;				/* User ID */
	Uint32   gid;				/* Group ID */
	char    *_Nullable loginClass;		/* Login class */
	char    *_Nullable gecos;		/* Honeywell login info */
	char    *_Nullable home;		/* Home directory */
	char    *_Nullable shell;		/* Default shell */
	char    *_Nullable tmp;			/* Temp. directory */
	AG_TAILQ_ENTRY(ag_user) users;
} AG_User;

/* List of user accounts */
typedef AG_TAILQ_HEAD(ag_user_list, ag_user) AG_UserList;

typedef struct ag_user_ops {
	const char *_Nonnull name;
	void (*_Nullable init) (void);
	void (*_Nullable destroy) (void);
	int  (*_Nonnull getUserByName) (AG_User    *_Nonnull _Restrict,
	                                const char *_Nonnull _Restrict);
	int  (*_Nonnull getUserByUID) (AG_User *_Nonnull, Uint32);
	int  (*_Nonnull getRealUser) (AG_User *_Nonnull);
	int  (*_Nonnull getEffectiveUser) (AG_User *_Nonnull);
} AG_UserOps;

#ifdef AG_USER

__BEGIN_DECLS
extern const AG_UserOps *_Nullable agUserOps;

/* Sync this list with AG_User(3) */
extern const AG_UserOps agUserOps_dummy;
extern const AG_UserOps agUserOps_posix;
extern const AG_UserOps agUserOps_win32;
extern const AG_UserOps agUserOps_xbox;

AG_User *_Nullable AG_UserNew(void); /* _Malloc_Like_Attribute; */
void               AG_UserFree(AG_User *_Nonnull);
void               AG_SetUserOps(const AG_UserOps *_Nonnull);
AG_User *_Nullable AG_GetUserByName(const char *_Nonnull);
AG_User *_Nullable AG_GetUserByUID(Uint32);
AG_User *_Nullable AG_GetRealUser(void);
AG_User *_Nullable AG_GetEffectiveUser(void);
__END_DECLS

#endif /* AG_USER */
#include <agar/core/close.h>
