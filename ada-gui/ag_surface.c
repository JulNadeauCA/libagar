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

AG_Surface *
agar_surface_video_rgb (unsigned int w, unsigned int h)
{
  return AG_SurfaceVideoRGB (w, h);
}

AG_Surface *
agar_surface_video_rgba (unsigned int w, unsigned int h)
{
  return AG_SurfaceVideoRGBA (w, h);
}

void
agar_surface_lock (AG_Surface *surf)
{
  AG_SurfaceLock (surf);
}

void
agar_surface_unlock (AG_Surface *surf)
{
  AG_SurfaceUnlock (surf);
}
