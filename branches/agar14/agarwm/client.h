/*	Public domain	*/

#define WM_CLIENT_NAME_MAX 64

typedef struct wm_client {
	struct ag_object obj;
	Uint flags;
#define WM_CLIENT_DEBUG	0x01				/* Debugging mode */

	char name[WM_CLIENT_NAME_MAX];			/* Application name */
} WM_Client;

__BEGIN_DECLS
extern AG_ObjectClass wmClientClass;
__END_DECLS
