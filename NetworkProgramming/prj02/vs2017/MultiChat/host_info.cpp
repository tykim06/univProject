#include "host_info.h"
#include <stdio.h>
#include <WS2tcpip.h>

static host_info_t *p_host_info;

void host_info_init(host_info_t *host_info) {
	p_host_info = host_info;

	sprintf(p_host_info->multi_cast_ip, "235.7.8.1");
	p_host_info->multi_cast_port = 9000;
	sprintf(p_host_info->name, "TY");
	
    p_host_info->send_time = time(NULL);
}

char *host_info_get_p_multicast_ip() {
	return p_host_info->multi_cast_ip;
}

unsigned short host_info_get_multicast_port() {
	return p_host_info->multi_cast_port;
}

char *host_info_get_p_name() {
	return p_host_info->name;
}

void host_info_set_name(char *name) {
	strcpy(p_host_info->name, name);
}

bool host_info_is_equal_name(char *name) {
	return strcmp(p_host_info->name, name) == 0;
}

time_t *host_info_get_p_time() {
	return &p_host_info->send_time;
}