/*	$Csoft: version.h,v 1.1 2002/02/18 00:54:37 vedge Exp $	*/

struct version {
	char	*name;
	int	 vermaj, vermin;
};

int	version_read(int, const struct version *);
int	version_write(int, const struct version *);

