/*	Public domain	*/

#define RCSINFO_MSG_MAX	16384
#define RCSINFO_NAME_MAX 1024			/* Sync with agar object.h */
#define RCSINFO_TYPE_MAX 32			/* Sync with agar object.h */
#define RCSINFO_AUTHOR_MAX USER_NAME_MAX
#define RCSINFO_DIGEST_MAX 160
#define RCS_CURRENT_REVISION 0

int rcs_commit(NS_Server *, NS_Command *, void *);
int rcs_info(NS_Server *, NS_Command *, void *);
int rcs_update(NS_Server *, NS_Command *, void *);
int rcs_list(NS_Server *, NS_Command *, void *);
int rcs_log(NS_Server *, NS_Command *, void *);

int rcsinfo_add_revision(const char *, struct user *, Uint *, const char *,
                         const char *, const char *, const char *);
int rcsinfo_get_revision(const char *, Uint *, char *, char *, char *, char *,
		         char *);
