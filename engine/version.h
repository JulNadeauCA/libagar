/*	$Csoft: version.h,v 1.2 2002/04/28 11:04:09 vedge Exp $	*/
/*	Public domain	*/

struct version {
	char	*name;
	int	 vermaj, vermin;
};

int	version_read(int, const struct version *);
int	version_write(int, const struct version *);

