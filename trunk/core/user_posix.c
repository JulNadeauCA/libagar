/*	Public domain	*/

#include <core/core.h>

#include <unistd.h>
#include <pwd.h>

#include <config/have_getpwnam_r.h>
#include <config/have_getenv.h>

/*
 * Access user account information on POSIX-like platforms.
 */

static void
ConvertUserData(AG_User *u, const struct passwd *pw)
{
	Strlcpy(u->name, pw->pw_name, sizeof(u->name));
	u->uid = (Uint32)pw->pw_uid;
	u->gid = (Uint32)pw->pw_gid;
	u->loginClass = TryStrdup(pw->pw_class);
	u->gecos = TryStrdup(pw->pw_gecos);
	u->home = TryStrdup(pw->pw_dir);
	u->shell = TryStrdup(pw->pw_shell);

#ifdef HAVE_GETENV
	{
		char *s;
		if ((s = getenv("TMPDIR")) != NULL && s[0] != '\0') {
			u->tmp = TryStrdup(s);
		} else {
			u->tmp = TryStrdup("/tmp");
		}
	}
#else
	u->tmp = TryStrdup("/tmp");
#endif
}

static int
GetUserByName(AG_User *u, const char *name)
{
#ifdef HAVE_GETPWNAM_R
	struct passwd pwStorage, *pwRes;
	char *buf;
	size_t bufSize;
	int rv;

	bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufSize == -1) {
		bufSize = 16384;
	}
	if ((buf = TryMalloc(bufSize)) == NULL) {
		return (-1);
	}
	rv = getpwnam_r(name, &pwStorage, buf, bufSize, &pwRes);
	if (pwRes == NULL) {
		if (rv == 0) {
			return (0);		/* User not found */
		} else {
			AG_SetError("getpwnam_r: %s", strerror(rv));
			return (-1);
		}
	}
	ConvertUserData(u, &pwStorage);
	free(buf);
#else
	struct passwd *pw = getpwnam(name);
	if (pw == NULL) {
		return (0);
	}
	ConvertUserData(u, pw);
#endif
	return (1);
}

static int
GetUserByUID(AG_User *u, Uint32 uid)
{
#ifdef HAVE_GETPWNAM_R
	struct passwd pwStorage, *pwRes;
	char *buf;
	size_t bufSize;
	int rv;

	bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufSize == -1) {
		bufSize = 16384;
	}
	if ((buf = TryMalloc(bufSize)) == NULL) {
		return (-1);
	}
	rv = getpwuid_r((uid_t)uid, &pwStorage, buf, bufSize, &pwRes);
	if (pwRes == NULL) {
		if (rv == 0) {
			return (0);		/* User not found */
		} else {
			AG_SetError("getpwnam_r: %s", strerror(rv));
			return (-1);
		}
	}
	ConvertUserData(u, &pwStorage);
	free(buf);
#else
	struct passwd *pw = getpwuid((uid_t)uid);
	if (pw == NULL) {
		return (0);
	}
	ConvertUserData(u, pw);
#endif
	return (1);
}

static int
GetRealUser(AG_User *u)
{
	return GetUserByUID(u, (Uint32)getuid());
}

static int
GetEffectiveUser(AG_User *u)
{
	return GetUserByUID(u, (Uint32)geteuid());
}

const AG_UserOps agUserOps_posix = {
	"posix",
	NULL,		/* init */
	NULL,		/* destroy */
	GetUserByName,
	GetUserByUID,
	GetRealUser,
	GetEffectiveUser,
};
