/*	$Csoft: version.h,v 1.3 2002/06/09 10:08:04 vedge Exp $	*/
/*	Public domain	*/

struct version {
	char	*name;
	Uint32	 vermaj, vermin;
};

int	version_read(int, const struct version *);
int	version_write(int, const struct version *);

