/*	Public domain	*/

#include <agar/core/begin.h>

#define AG_USER_NAME_MAX 73

/* User account (POSIX-style, or otherwise) */
typedef struct ag_user {
	char	 name[AG_USER_NAME_MAX];	/* User name */
	Uint     flags;
#define AG_USER_NO_ACCOUNT	0x01		/* Not a real user account */
	char    *passwd;			/* Encrypted password */
	Uint32   uid;				/* User ID */
	Uint32   gid;				/* Group ID */
	char    *loginClass;			/* Login class */
	char    *gecos;				/* Honeywell login info */
	char    *home;				/* Home directory */
	char    *shell;				/* Default shell */
	char    *tmp;				/* Temp. directory */
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

static __inline__ int
AG_GetUserByName(AG_User *u, const char *name)
{
	return agUserOps->getUserByName(u, name);
}
static __inline__ int
AG_GetUserByUID(AG_User *u, Uint32 uid)
{
	return agUserOps->getUserByUID(u, uid);
}
static __inline__ int
AG_GetRealUser(AG_User *u)
{
	return agUserOps->getRealUser(u);
}
static __inline__ int
AG_GetEffectiveUser(AG_User *u)
{
	return agUserOps->getEffectiveUser(u);
}
__END_DECLS

#include <agar/core/close.h>
