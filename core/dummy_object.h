/*	Public domain	*/

/* Dummy object instance */
typedef struct ag_dummy {
	struct ag_object _inherit;	/* AG_Object -> AG_Dummy */
	Uint flags;
#define AG_DUMMY_OPTION1 0x01		/* Some option */
#define AG_DUMMY_OPTION2 0x02		/* Some other option */
#define AG_DUMMY_TMPFLAG 0x04		/* Some non-persistent flag */
#define AG_DUMMY_SAVED   (AG_DUMMY_OPTION1 | AG_DUMMY_OPTION2)
	int x,y;			/* Some integers */
	void *myData;			/* Some dynamically allocated data */
	/* ... */
} AG_Dummy;

#define AGDUMMY(obj)            ((AG_Dummy *)(obj))
#define AGCDUMMY(obj)           ((const AG_Dummy *)(obj))
#define AG_DUMMY_SELF()          AGDUMMY( AG_OBJECT(0,"AG_Dummy:*") )
#define AG_DUMMY_PTR(n)          AGDUMMY( AG_OBJECT((n),"AG_Dummy:*") )
#define AG_DUMMY_NAMED(n)        AGDUMMY( AG_OBJECT_NAMED((n),"AG_Dummy:*") )
#define AG_CONST_DUMMY_SELF()   AGCDUMMY( AG_CONST_OBJECT(0,"AG_Dummy:*") )
#define AG_CONST_DUMMY_PTR(n)   AGCDUMMY( AG_CONST_OBJECT((n),"AG_Dummy:*") )
#define AG_CONST_DUMMY_NAMED(n) AGCDUMMY( AG_CONST_OBJECT_NAMED((n),"AG_Dummy:*") )

extern AG_ObjectClass agDummyClass;	/* Class description */

AG_Dummy *_Nullable AG_DummyNew(int,int, Uint);		/* Optional */
