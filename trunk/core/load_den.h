/*	Public domain	*/

#ifndef	_AGAR_CORE_LOAD_DEN_H_
#define	_AGAR_CORE_LOAD_DEN_H_
#include <agar/core/begin.h>

#define AG_DEN_NAME_MAX		64
#define AG_DEN_HINT_MAX		16
#define AG_DEN_MEMBER_NAME_MAX	64
#define AG_DEN_MEMBER_LANG_MAX	4
#define AG_DEN_MAPPING_SIZE	(sizeof(Uint32) + AG_DEN_MEMBER_NAME_MAX + \
				 sizeof(Uint32) + AG_DEN_MEMBER_LANG_MAX + \
				 sizeof(Uint32)*2)

typedef struct ag_den_member {
	char	 name[AG_DEN_MEMBER_NAME_MAX];	/* Identifier */
	char	 lang[AG_DEN_MEMBER_LANG_MAX];	/* ISO 639 language code */
	off_t	 offs;				/* Position in archive */
	size_t	 size;				/* Size in bytes */
} AG_DenMember;

typedef struct ag_den {
	AG_DataSource *buf;		/* Data source */
	char hint[AG_DEN_HINT_MAX];	/* Hint on type of content */
	char name[AG_DEN_NAME_MAX];	/* Name of the package */
	
	char *author;			/* Author of the contents */
	char *copyright;		/* Copyright notice */
	char *descr;			/* Description of contents */
	char *keywords;			/* Comma-separated keywords */

	AG_DenMember *members;
	Uint32 nmembers;
	off_t mapoffs;		/* Offset of mapping table */
} AG_Den;

enum ag_den_open_mode {
	AG_DEN_READ,
	AG_DEN_WRITE
};

__BEGIN_DECLS
AG_Den	*AG_DenOpen(const char *, enum ag_den_open_mode);
void	 AG_DenClose(AG_Den *);

void	AG_DenWriteHeader(AG_Den *, int);
void	AG_DenWriteMappings(AG_Den *);

int	AG_DenImportFile(AG_Den *, int, const char *, const char *,
	                const char *);
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_LOAD_DEN_H_ */
