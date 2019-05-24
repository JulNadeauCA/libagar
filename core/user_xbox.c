/*	Public domain	*/

/*
 * User information on the Xbox console.
 */

#include <agar/core/core.h>
#ifdef AG_USER

static int
GetUserByUID(AG_User *_Nonnull u, Uint32 uid)
{
	if (AG_XBOX_DriveIsMounted('T')) {
		if ((u->home = TryStrdup("T:\\")) == NULL ||
		    (u->tmp = TryStrdup("T:\\temp")) == NULL)
			return (-1);
	} else {
		if ((u->home = TryStrdup("D:\\")) == NULL ||
		    (u->tmp = TryStrdup("D:\\temp")) == NULL)
			return (-1);
	}
	u->flags |= AG_USER_NO_ACCOUNT;			/* Not a real user */
	return (0);
}

static int
GetUserByName(AG_User *_Nonnull u, const char *_Nonnull name)
{
	return GetUserByUID(u, 0);
}

static int
GetRealUser(AG_User *_Nonnull u)
{
	return GetUserByUID(u, 0);
}

static int
GetEffectiveUser(AG_User *_Nonnull u)
{
	return GetUserByUID(u, 0);
}

const AG_UserOps agUserOps_xbox = {
	"xbox",
	NULL,		/* init */
	NULL,		/* destroy */
	GetUserByName,
	GetUserByUID,
	GetRealUser,
	GetEffectiveUser
};

#endif /* AG_USER */
