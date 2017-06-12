#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

bool is_valid_ip(char *ipaddr) {
	long ip = inet_addr(ipaddr);
	if ((ip & 0xff) < 0 || (ip & 0xff) > 223) return false;
	ip = ip >> 8;
	if ((ip & 0xff) > 255 || (ip & 0xff) < 0) return false;
	ip = ip >> 8;
	if ((ip & 0xff) > 255 || (ip & 0xff) < 0) return false;
	ip = ip >> 8;
	if ((ip & 0xff) > 255 || (ip & 0xff) < 0) return false;

	return true;
}

bool is_valid_port(char *c_port, unsigned short *port) {
	unsigned short u_s_port;
	if (strlen(c_port) > 5) return false;
	if (sscanf(c_port, "%hu", &u_s_port)) {
		if (u_s_port >= 0 && u_s_port <= 65535) {
			*port = u_s_port;
			return true;
		}
	}
	return false;
}

bool is_valid_nickname(char *nickname) {
	if (strlen(nickname) == 0) return false;
	
	return true;
}