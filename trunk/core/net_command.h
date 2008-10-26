/*	Public domain	*/

#include <agar/core/begin.h>

typedef struct ns_command_arg {
	char	 key[32];
	void	*value;
	size_t	 size;
} NS_CommandArg, NC_CommandArg;

typedef struct ns_command {
	char		 name[64];
	NS_CommandArg	*args;
	int		nargs;
} NS_Command, NC_Command;

__BEGIN_DECLS
void	 NS_InitCommand(NS_Command *);
void	 NS_DestroyCommand(NS_Command *);
char	*NS_CommandString(NS_Command *, const char *);
int	 NS_CommandInt(NS_Command *, const char *, int *);
int	 NS_CommandLong(NS_Command *, const char *, long *);
void	 NS_CommandCopyString(char *, NS_Command *, const char *, size_t);

#define NC_InitCommand		NS_InitCommand
#define NC_DestroyCommand	NS_DestroyCommand
#define NC_CommandString	NS_CommandString
#define NC_CommandInt		NS_CommandInt
#define NC_CommandLong		NS_CommandLong
#define NC_CommandCopyString	NS_CommandCopyString
__END_DECLS

#include <agar/core/close.h>
