/*	Public domain	*/

#include <core/core.h>

static int
GetUserByName(AG_User *u, const char *name)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
GetUserByUID(AG_User *u, Uint32 uid)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
GetRealUser(AG_User *u)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
}

static int
GetEffectiveUser(AG_User *u)
{
	AG_SetError(_("Operation not supported"));
	return (-1);
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
