/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sg_dummy {
	struct sg_node node;
	SG_Real foo;
} SG_Dummy;

__BEGIN_DECLS
extern SG_NodeOps sgDummyOps;

SG_Dummy	*SG_DummyNew(void *, const char *);
void		 SG_DummyInit(void *, const char *);
int		 SG_DummyLoad(void *, AG_Netbuf *);
int		 SG_DummySave(void *, AG_Netbuf *);
void		 SG_DummyEdit(void *, AG_Widget *, SG_View *);
void		 SG_DummyDraw(void *, SG_View *);
__END_DECLS

