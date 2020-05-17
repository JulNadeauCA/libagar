/*	Public domain	*/

#include <agar/core/core.h>
#ifdef AG_USER

/* Lookup a user account by numerical UID */
static int
GetUserByUID(AG_User *u, Uint32 uid)
{
	AG_SetError("Operation not supported");
	return (-1);
}

/* Lookup a user account by name */
static int
GetUserByName(AG_User *u, const char *name)
{
	return GetUserByUID(u, 0);
}

/* Lookup the user associated with real UID of running process */
static int
GetRealUser(AG_User *u)
{
	return GetUserByUID(u, 0);
}

/* Lookup the user associated with effective UID of running process */
static int
GetEffectiveUser(AG_User *u)
{
	return GetUserByUID(u, 0);
}

const AG_UserOps agUserOps_dummy = {
	"dummy",
	NULL,		/* init */
	NULL,		/* destroy */
	GetUserByName,
	GetUserByUID,
	GetRealUser,
	GetEffectiveUser
};
#endif /* AG_USER */
