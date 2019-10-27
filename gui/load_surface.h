/*	Public domain	*/

#include <agar/gui/begin.h>

__BEGIN_DECLS
AG_Surface *_Nullable AG_ReadSurface(AG_DataSource *_Nonnull);
void                  AG_WriteSurface(AG_DataSource *_Nonnull,
                                      AG_Surface *_Nonnull);

int  AG_ReadPalette(AG_Palette *_Nonnull, AG_DataSource *_Nonnull);
void AG_WritePalette(AG_DataSource *_Nonnull, const AG_Palette *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
