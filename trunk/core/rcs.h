/*	$Csoft: rcs.h,v 1.8 2005/09/20 13:46:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_RCSMGR_H_
#define _AGAR_RCSMGR_H_

#include <agar/config/network.h>

#include "begin_code.h"

extern char agRcsHostname[64];
extern char agRcsUsername[32];
extern char agRcsPassword[32];
extern Uint agRcsPort;
extern int agRcsMode;

enum ag_rcs_status {
	AG_RCS_ERROR,	 /* Status unavailable */
	AG_RCS_UNKNOWN,	 /* Not on the repository */
	AG_RCS_UPTODATE, /* Working copy = last rev and checksum matches */
	AG_RCS_LOCALMOD, /* Working copy = last rev and checksum mismatch */
	AG_RCS_DESYNCH,	 /* Working copy < last rev */
};

struct ag_tlist;

__BEGIN_DECLS
#ifdef NETWORK
void AG_RcsInit(void);
void AG_RcsDestroy(void);
int AG_RcsConnect(void);
void AG_RcsDisconnect(void);

int AG_RcsUpdate(AG_Object *);
int AG_RcsUpdateAll(AG_Object *);
int AG_RcsCommit(AG_Object *);
int AG_RcsCommitAll(AG_Object *);
int AG_RcsImport(AG_Object *);
int AG_RcsImportAll(AG_Object *);

int AG_RcsGetWorkingRev(AG_Object *, Uint *);
int AG_RcsSetWorkingRev(AG_Object *, Uint);
enum ag_rcs_status AG_RcsStatus(AG_Object *, const char *, const char *,
		                char *, char *, Uint *, Uint *);
int AG_RcsLog(const char *, struct ag_tlist *);
int AG_RcsList(struct ag_tlist *);
int AG_RcsDelete(const char *);
int AG_RcsRename(const char *, const char *);
int AG_RcsCheckout(const char *);
#endif /* NETWORK */
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_RCSMGR_H_ */
