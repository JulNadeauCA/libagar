#include <agar/core.h>

void
agar_dso_lock (void)
{
  AG_LockDSO();
}

void
agar_dso_unlock (void)
{
  AG_UnlockDSO();
}
