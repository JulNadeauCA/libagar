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

/* Sync this list with AG_User(3) */
extern const AG_UserOps  agUserOps_dummy;
extern const AG_UserOps  agUserOps_posix;
extern const AG_UserOps  agUserOps_win32;
extern const AG_UserOps  agUserOps_xbox;

AG_User  *AG_UserNew(void);
void      AG_UserFree(AG_User *);
void      AG_SetUserOps(const AG_UserOps *);

static __inline__ AG_User *
AG_GetUserByName(const char *name)
{
	AG_User *u;

	if ((u = AG_UserNew()) == NULL) {
		return (NULL);
	}
	if (agUserOps->getUserByName(u, name) == -1) {
		AG_UserFree(u);
		return (NULL);
	}
	return (u);
}
static __inline__ AG_User *
AG_GetUserByUID(Uint32 uid)
{
	AG_User *u;

	if ((u = AG_UserNew()) == NULL) {
		return (NULL);
	}
	if (agUserOps->getUserByUID(u, uid) == -1) {
		AG_UserFree(u);
		return (NULL);
	}
	return (u);
}
static __inline__ AG_User *
AG_GetRealUser(void)
{
	AG_User *u;

	if ((u = AG_UserNew()) == NULL) {
		return (NULL);
	}
	if (agUserOps->getRealUser(u) == -1) {
		AG_UserFree(u);
		return (NULL);
	}
	return (u);
}
static __inline__ AG_User *
AG_GetEffectiveUser(void)
{
	AG_User *u;

	if ((u = AG_UserNew()) == NULL) {
		return (NULL);
	}
	if (agUserOps->getEffectiveUser(u) == -1) {
		AG_UserFree(u);
		return (NULL);
	}
	return (u);
}
__END_DECLS

#include <agar/core/close.h>
