/*	Public domain	*/

#include <netinet/in.h>

union sockunion {
	struct sockinet {
		unsigned char	si_len;
		sa_family_t	si_family;
		in_port_t	si_port;
	} su_si;
	struct sockaddr_in	su_sin;
	struct sockaddr_in6	su_sin6;
};
#define su_len		su_si.si_len
#define su_family	su_si.si_family
#define su_port		su_si.si_port

