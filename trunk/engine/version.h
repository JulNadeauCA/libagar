/*	$Csoft: version.h,v 1.6 2003/02/22 11:49:53 vedge Exp $	*/
/*	Public domain	*/

struct version {
	char	*name;
	Uint32	 major;
	Uint32	 minor;
};

#define VERSION_USER_MAX	128
#define VERSION_HOST_MAX	256

int	version_read(struct netbuf *, const struct version *, struct version *);
void	version_write(struct netbuf *, const struct version *);

