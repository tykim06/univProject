#include "host_info.h"
#include <stdio.h>
#include <WS2tcpip.h>

static host_info_t *p_host_info;

bool is_class_d_ip(void) {
	long ip = inet_addr(p_host_info->multi_cast_ip);
	if((ip&0xff) < 224 || (ip&0xff) > 239) return false;
	ip = ip >> 8;
	if((ip&0xff) > 255 || (ip&0xff) < 0) return false;
	ip = ip >> 8;
	if((ip&0xff) > 255 || (ip&0xff) < 0) return false;
	ip = ip >> 8;
	if((ip&0xff) > 255 || (ip&0xff) < 0) return false;

	return true;
}

void host_info_init(host_info_t *host_info) {
	p_host_info = host_info;

	/*
	do {
		printf("Enter multicast ip: ");
		scanf("%s",p_host_info->multi_cast_ip);
		fflush(stdin);
	} while(!is_class_d_ip());	
	do {
		printf("Enter multicast port: ");
		fflush(stdin);
	} while(scanf("%ld",&p_host_info->multi_cast_port) != 1
		|| p_host_info->multi_cast_port<0
		|| p_host_info->multi_cast_port>65535
		|| getchar() != '\n');
	*/
	sprintf(p_host_info->multi_cast_ip, "235.7.8.1");
	p_host_info->multi_cast_port = 9000;
	printf("enter your nickname: ");
	scanf("%s",p_host_info->name);
	fflush(stdin);
	printf("start multicast chat! ip: %s, port: %d, name: %s\n", p_host_info->multi_cast_ip, p_host_info->multi_cast_port, p_host_info->name);
	printf("More information Enter :help\n\n\n");
	
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