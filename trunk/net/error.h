/*	Public domain	*/

__BEGIN_DECLS
const char	*AGN_GetError(void);
void		 AGN_SetError(const char *, ...);
void		*AGN_Malloc(size_t);
void		*AGN_Realloc(void *, size_t);
__END_DECLS
