/*	$Csoft: version.h,v 1.5 2003/01/17 06:52:19 vedge Exp $	*/
/*	Public domain	*/

struct version {
	char	*name;
	Uint32	 major;
	Uint32	 minor;
};

int	version_read(int, const struct version *, struct version *);
void	version_write(int, const struct version *);
void	version_buf_write(struct fobj_buf *, const struct version *);

