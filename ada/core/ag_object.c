#include <agar/core.h>

int   ag_of_class(void *o, const char *p) { return AG_OfClass(o, p); }
void *ag_object_find_child(void *p, const char *n) { return AG_ObjectFindChild(p, n); }
void  ag_lock_vfs(void *o) { AG_LockVFS(o); }
void  ag_unlock_vfs(void *o) { AG_UnlockVFS(o); }

