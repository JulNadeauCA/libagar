/*	$Csoft: dummy.h,v 1.11 2002/06/09 10:08:08 vedge Exp $	*/
/*	Public domain	*/

struct dummy {
	struct	 widget wid;

	char	 *caption;
	int	 flags;
#define DUMMY_FOO	0x01
};

struct dummy	*dummy_new(struct region *, const char *, int);
void		 dummy_init(struct dummy *, const char *, int);
void	 	 dummy_destroy(void *);
void		 dummy_draw(void *);

