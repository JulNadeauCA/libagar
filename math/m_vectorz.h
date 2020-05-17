/*	Public domain	*/

typedef struct m_vectorz {
	Uint n;
	Uint32 _pad;
	int *_Nonnull v;
} M_VectorZ;

__BEGIN_DECLS
M_VectorZ *_Nonnull M_VectorNewZ(Uint);

void M_VectorSetZ(M_VectorZ *_Nonnull, int);
void M_VectorCopyZ(const M_VectorZ *_Nonnull, M_VectorZ *_Nonnull);
void M_VectorFreeZ(M_VectorZ *_Nonnull);
void M_VectorAddZv(M_VectorZ *_Nonnull, const M_VectorZ *_Nonnull);
void M_VectorSubZv(M_VectorZ *_Nonnull, const M_VectorZ *_Nonnull);
void M_VectorScaleZv(M_VectorZ *_Nonnull, M_Real);
void M_VectorResizeV(M_VectorZ *_Nonnull, Uint);
void M_VectorPrintZ(const M_VectorZ *_Nonnull);
__END_DECLS
