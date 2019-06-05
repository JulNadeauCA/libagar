#include <agar/core.h>
#include <agar/gui.h>

AG_Surface *
agar_surface_std_rgb (unsigned int w, unsigned int h)
{
  return AG_SurfaceStdRGB (w, h);
}

AG_Surface *
agar_surface_std_rgba (unsigned int w, unsigned int h)
{
  return AG_SurfaceStdRGBA (w, h);
}
