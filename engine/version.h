/*	$Csoft: version.h,v 1.4 2002/09/04 03:19:02 vedge Exp $	*/
/*	Public domain	*/

struct version {
	char	*name;
	Uint32	 vermaj, vermin;
};

int	version_read(int, const struct version *);
void	version_write(int, const struct version *);
void	version_buf_write(struct fobj_buf *, const struct version *);

