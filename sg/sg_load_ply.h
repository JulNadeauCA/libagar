/*	Public domain	*/

#define SG_PLY_LOAD_VTX_NORMALS	0x01	/* Load vertex normals */
#define SG_PLY_LOAD_VTX_COLORS	0x02	/* Load vertex colors */
#define SG_PLY_LOAD_TEXCOORDS	0x04	/* Load texture coordinates */
#define SG_PLY_DUP_VERTICES	0x08	/* Eliminate duplicate vertices */

__BEGIN_DECLS
int SG_ObjectLoadPLY(void *, const char *, Uint);
__END_DECLS
