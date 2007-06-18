/*	Public domain	*/

typedef struct sg_box {
	struct sg_object obj;
} SG_Box;

__BEGIN_DECLS
extern SG_NodeOps sgBoxOps;

SG_Box	*SG_BoxNew(void *, const char *);
void	 SG_BoxInit(void *, const char *);
int	 SG_BoxLoad(void *, AG_Netbuf *);
int	 SG_BoxSave(void *, AG_Netbuf *);
__END_DECLS
