/*	$Csoft: real.h,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

#define DEN_NAME_MAX		64
#define DEN_HINT_MAX		16
#define DEN_MEMBER_NAME_MAX	64
#define DEN_MEMBER_LANG_MAX	4
#define DEN_MAPPING_SIZE	(sizeof(Uint32) + DEN_MEMBER_NAME_MAX + \
				 sizeof(Uint32) + DEN_MEMBER_LANG_MAX + \
				 sizeof(Uint32)*2)

struct den_member {
	char	 name[DEN_MEMBER_NAME_MAX];	/* Identifier */
	char	 lang[DEN_MEMBER_LANG_MAX];	/* ISO 639 language code */
	off_t	 offs;				/* Position in archive */
	size_t	 size;				/* Size in bytes */
};

struct den {
	struct netbuf	*buf;			/* I/O buffer */
	char		 hint[DEN_HINT_MAX];	/* Hint on type of content */
	char		 name[DEN_NAME_MAX];	/* Name of the package */
	
	char		*author;		/* Author of the contents */
	char		*copyright;		/* Copyright notice */
	char		*descr;			/* Description of contents */
	char		*keywords;		/* Comma-separated keywords */

	struct den_member *members;
	Uint32		  nmembers;
	off_t		   mapoffs;		/* Offset of mapping table */
};

enum den_open_mode {
	DEN_READ,
	DEN_WRITE
};

__BEGIN_DECLS
struct den	*den_open(const char *, enum den_open_mode);
struct netbuf	*den_open_member(struct den *, int);
void		 den_close(struct den *);

void	den_write_header(struct den *, int);
void	den_write_mappings(struct den *);

int	den_import_file(struct den *, int, const char *, const char *,
	                const char *);
__END_DECLS

#include "close_code.h"
