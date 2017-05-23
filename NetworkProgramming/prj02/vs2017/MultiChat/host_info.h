#ifndef __HOST_INFO__
#define __HOST_INFO__

#include <time.h>

#define HOST_INFO_BUFSIZE 30

typedef struct host_info_s {
	char multi_cast_ip[HOST_INFO_BUFSIZE];
	unsigned short multi_cast_port;
	char name[HOST_INFO_BUFSIZE];
	time_t send_time;
} host_info_t;

void host_info_init(char *ip, unsigned short  port, char *name);

char *host_info_get_p_multicast_ip();

unsigned short host_info_get_multicast_port();

char *host_info_get_p_name();
void host_info_set_name(char *name);
bool host_info_is_equal_name(char *name);

char *host_info_get_current_time();

#endif