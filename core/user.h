/*	Public domain	*/

#include <agar/core/begin.h>

#define AG_USER_NAME_MAX 73

/* User account (POSIX-style, or otherwise) */
typedef struct ag_user {
	char	 name[AG_USER_NAME_MAX];	/* User name */
	Uint     flags;
	char    *passwd;			/* Encrypted password */
	Uint32   uid;				/* User ID */
	Uint32   gid;				/* Group ID */
	char    *loginClass;			/* Login class */
	char    *gecos;				/* Honeywell login info */
	char    *home;				/* Home directory */
	char    *shell;				/* Default shell */
	AG_TAILQ_ENTRY(ag_user) users;
} AG_User;

/* List of user accounts */
typedef AG_TAILQ_HEAD(ag_user_list, ag_user) AG_UserList;

typedef struct ag_user_ops {
	const char *name;
	void     (*init)(void);
	void     (*destroy)(void);
	int      (*getUserByName)(AG_User *, const char *);
	int      (*getUserByUID)(AG_User *, Uint32);
	int      (*getRealUser)(AG_User *);
	int      (*getEffectiveUser)(AG_User *);
} AG_UserOps;

__BEGIN_DECLS
extern const AG_UserOps *agUserOps;
extern const AG_UserOps  agUserOps_posix;
extern const AG_UserOps  agUserOps_dummy;

AG_User  *AG_UserNew(void);
void      AG_UserFree(AG_User *);
void      AG_SetUserOps(const AG_UserOps *);

#define   AG_GetUserByName	agUserOps->getUserByName
#define   AG_GetUserByUID	agUserOps->getUserByUID
#define   AG_GetRealUser	agUserOps->getRealUser
#define   AG_GetEffectiveUser   agUserOps->getEffectiveUser
__END_DECLS

#include <agar/core/close.h>
