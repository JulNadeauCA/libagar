/*	Public domain	*/

typedef struct agn_command_arg {
	char	 key[32];
	void	*value;
	size_t	 size;
} AGN_CommandArg;

typedef struct agn_command {
	char		 name[64];
	AGN_CommandArg	*args;
	int		nargs;
} AGN_Command;

__BEGIN_DECLS
void	 AGN_InitCommand(AGN_Command *);
void	 AGN_DestroyCommand(AGN_Command *);
char	*AGN_CommandString(AGN_Command *, const char *);
int	 AGN_CommandInt(AGN_Command *, const char *, int *);
int	 AGN_CommandLong(AGN_Command *, const char *, long *);
void	 AGN_CommandCopyString(char *, AGN_Command *, const char *, size_t);
__END_DECLS
