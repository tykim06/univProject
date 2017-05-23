#include "host_info.h"
#include <stdio.h>
#include <WS2tcpip.h>

static host_info_t host_info;

void host_info_init(char *ip, unsigned short  port, char *name) {
	strcpy(host_info.multi_cast_ip, ip);
	host_info.multi_cast_port = port;
	strcpy(host_info.name, name);
}

char *host_info_get_p_multicast_ip() {
	return host_info.multi_cast_ip;
}

unsigned short host_info_get_multicast_port() {
	return host_info.multi_cast_port;
}

char *host_info_get_p_name() {
	return host_info.name;
}

void host_info_set_name(char *name) {
	strcpy(host_info.name, name);
}

bool host_info_is_equal_name(char *name) {
	return strcmp(host_info.name, name) == 0;
}

char *host_info_get_current_time() {
	host_info.send_time = time(NULL);
	return ctime(&host_info.send_time);
}