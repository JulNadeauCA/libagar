/*	Public domain	*/

#include <core/core.h>

static int
GetUserByUID(AG_User *u, Uint32 uid)
{
	if ((u->home = TryStrdup(".")) == NULL ||
	    (u->tmp = TryStrdup("tmp")) == NULL) {
		return (-1);
	}

	u->flags |= AG_USER_NO_ACCOUNT;			/* Not a real user */
	return (0);
}

static int
GetUserByName(AG_User *u, const char *name)
{
	return GetUserByUID(u, 0);
}

static int
GetRealUser(AG_User *u)
{
	return GetUserByUID(u, 0);
}

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