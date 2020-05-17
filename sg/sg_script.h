/*	Public domain	*/

/* Instruction type */
enum sg_script_insn_type {
	SG_INSN_NOOP,		/* No-op */
	SG_INSN_CREATE,		/* Create node (object data follows) */
	SG_INSN_DELETE,		/* Delete target node */
	SG_INSN_ACTION,		/* Execute SG_Action */
	SG_INSN_CAMACTION,	/* Execute SG_Action on active camera */
	SG_INSN_LAST
};

/* Instruction interpolation modes. */
enum sg_script_interp_mode {
	SG_SCRIPT_INTERP_NONE,		/* No interpolation */
	SG_SCRIPT_INTERP_LINEAR		/* Linear interpolation */
};

typedef struct sg_script_insn {
	enum sg_script_insn_type type;		/* Instruction type */
	Uint flags;
#define SG_SCRIPT_INSN_SUPPRESS	0x01		/* Suppress instruction */
#define SG_SCRIPT_INSN_SELECTED	0x02		/* Selected in editor */
#define SG_SCRIPT_INSN_SAVED	(SG_SCRIPT_INSN_SUPPRESS)
	char *_Nullable tgtName;		/* Target node path */
	union {
		struct {
			char *_Nullable name;		/* Name (under parent) */
			AG_ObjectClass *_Nullable cls;	/* Node class */
			void *_Nullable data;		/* Saved dataset */
			AG_Size size;			/* Dataset size */
#if AG_MODEL == AG_MEDIUM
			Uint32 _pad;
#endif
		} create; 
		SG_Action action;
#ifdef _AGAR_SG_INTERNAL
#define si_create args.create
#define si_action args.action
#endif
	} args;
	AG_TAILQ_ENTRY(sg_script_insn) insns;
} SG_ScriptInsn;

typedef struct sg_script_frame {
	AG_TAILQ_HEAD(sg_script_insnsq,sg_script_insn) insns;
} SG_ScriptFrame;

typedef struct sg_script {
	struct ag_object _inherit;		/* AG_Object -> SG_Script */
	Uint flags;
#define SG_SCRIPT_SAVED 0
	int fps;				/* Frames/second */
	SG_ScriptFrame *_Nullable frames;	/* Time frames */
	Uint n;					/* Number of allocated frames */
	int t;					/* Current frame */
	int tFirst;				/* First frame index */
	int tLast;				/* Last frame index */
	int tPrev;				/* Previous frame */
	Uint32 _pad;
} SG_Script;

#define SGSCRIPT(obj)            ((SG_Script *)(obj))
#define SGCSCRIPT(obj)           ((const SG_Script *)(obj))
#define SG_SCRIPT_SELF()          SGSCRIPT( AG_OBJECT(0,"SG_Script:*") )
#define SG_SCRIPT_PTR(n)          SGSCRIPT( AG_OBJECT((n),"SG_Script:*") )
#define SG_SCRIPT_NAMED(n)        SGSCRIPT( AG_OBJECT_NAMED((n),"SG_Script:*") )
#define SG_CONST_SCRIPT_SELF()   SGCSCRIPT( AG_CONST_OBJECT(0,"SG_Script:*") )
#define SG_CONST_SCRIPT_PTR(n)   SGCSCRIPT( AG_CONST_OBJECT((n),"SG_Script:*") )
#define SG_CONST_SCRIPT_NAMED(n) SGCSCRIPT( AG_CONST_OBJECT_NAMED((n),"SG_Script:*") )

struct sg_view;

__BEGIN_DECLS
extern AG_ObjectClass sgScriptClass;
extern const char *_Nonnull sgScriptInsnNames[];

SG_Script *_Nonnull SG_ScriptNew(void *_Nullable, const char *_Nullable);
int                 SG_ScriptAlloc(SG_Script *_Nonnull, Uint);

SG_ScriptInsn *_Nullable SG_ScriptInsnNew(SG_Script *_Nonnull);
void                     SG_ScriptInsnFree(SG_ScriptInsn *_Nonnull);

int SG_ScriptAddInsn(SG_Script *_Nonnull, Uint, SG_ScriptInsn *_Nonnull);
int SG_ScriptAddInsnBefore(SG_Script *_Nonnull, Uint, SG_ScriptInsn *_Nonnull,
                           SG_ScriptInsn *_Nonnull);
int  SG_ScriptDelInsn(SG_Script *_Nonnull, Uint, SG_ScriptInsn *_Nonnull);
void SG_ScriptPrintInsn(const SG_ScriptInsn *_Nonnull, char *_Nonnull, AG_Size);
__END_DECLS
