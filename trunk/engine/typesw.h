/*	$Csoft$	*/
/*	Public domain	*/

struct object_type {
	char	*type;
	char	*desc;
	size_t	 size;
	const struct object_ops *ops;
};

extern const struct object_type	 typesw[];
extern const int		ntypesw;

