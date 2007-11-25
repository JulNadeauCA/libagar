/*	Public domain	*/

#define USER_NAME_MAX		64
#define USER_PASS_MAX		64
#define USER_REAL_NAME_MAX	64
#define USER_EMAIL_MAX		128
#define ISO639_LANG_MAX		12
#define ISO3166_COUNTRY_MAX	12
#define USER_COMMENTS_MAX	128

typedef struct user {
	struct ag_object obj;

	char name[USER_NAME_MAX];
	char pass[USER_PASS_MAX];
	char real_name[USER_REAL_NAME_MAX];
	char email[USER_EMAIL_MAX];
	char lang[ISO639_LANG_MAX];
	char country[ISO3166_COUNTRY_MAX];
	char comments[USER_COMMENTS_MAX];

	u_int flags;
#define USER_WRITE		0x01		/* Write access on repository */
#define USER_ADMIN		0x02		/* Administrative access */
#define USER_EMAIL_NOTICES	0x04		/* Send email notices */
#define USER_SETTABLE_FLAGS	(USER_EMAIL_NOTICES)

	AG_TAILQ_ENTRY(user) usert;		/* Entry in hash table */
	AG_TAILQ_ENTRY(user) userl;		/* Entry in flat list */
} User;

extern AG_ObjectClass UserClass;

void UserTblInit(void);
void UserTblRehash(void);
void UserTblDestroy(void);

User *UserLookup(const char *);
void  UserLink(struct user *);
void  UserUnlink(struct user *);

int user_get_infos(NS_Command *, void *);
int user_set_infos(NS_Command *, void *);
int user_activate(NS_Command *, void *);
int user_show(NS_Command *, void *);
