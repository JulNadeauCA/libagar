/*	$Csoft: rcs.h,v 1.7 2005/09/17 15:22:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RCSMGR_H_
#define _AGAR_RCSMGR_H_

#include "begin_code.h"

extern char rcs_hostname[64];
extern char rcs_username[32];
extern char rcs_password[32];
extern u_int rcs_port;
extern int rcs;

enum rcs_status {
	RCS_ERROR,	/* Status unavailable */
	RCS_UNKNOWN,	/* Not on the repository */
	RCS_UPTODATE,	/* Working copy = last revision and checksum matches */
	RCS_LOCALMOD,	/* Working copy = last revision and checksum mismatch */
	RCS_DESYNCH,	/* Working copy < last revision */
};

struct tlist;

__BEGIN_DECLS
#ifdef NETWORK
void rcs_init(void);
void rcs_destroy(void);
int rcs_connect(void);
void rcs_disconnect(void);

int rcs_update(struct object *);
int rcs_update_all(struct object *);
int rcs_commit(struct object *);
int rcs_commit_all(struct object *);
int rcs_import(struct object *);
int rcs_import_all(struct object *);

int rcs_get_working_rev(struct object *, u_int *);
int rcs_set_working_rev(struct object *, u_int);
enum rcs_status rcs_status(struct object *, const char *, const char *,
		           char *, char *, u_int *, u_int *);
int rcs_log(const char *, struct tlist *);
int rcs_list(struct tlist *);
int rcs_delete(const char *);
int rcs_rename(const char *, const char *);
int rcs_checkout(const char *);
#endif /* NETWORK */
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RCSMGR_H_ */
