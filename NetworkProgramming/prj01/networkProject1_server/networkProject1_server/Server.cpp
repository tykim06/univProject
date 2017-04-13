// 2017�� 1�б� ��Ʈ��ũ���α׷��� ���� 1�� ����

// ����: ���¿� �й�: 092262

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    1024

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left > 0){
		received = recv(s, ptr, left, flags);
		if(received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if(received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

void set_domain_info(char *buf)
{
	int len = 0;

	HOSTENT *ptr = gethostbyname(buf);
	if(ptr == NULL) {
		err_display("gethostbyname()");
		sprintf(buf, "### Wrong Domain ###\n");
		return;
	}

	len = sprintf(buf, "### Official Domain Name ###\n");
	char *ptr1 = ptr->h_name;
	len += sprintf(&buf[len], "%s\n", ptr1);

	len += sprintf(&buf[len], "\n### Aliases ###\n");
	char **ptr2 = ptr->h_aliases;
	while(*ptr2){
		len += sprintf(&buf[len], "%s\n", *ptr2);
		++ptr2;
	}

	len += sprintf(&buf[len], "\n### IP addresses ###\n");
	IN_ADDR addr;
	char **ptr3 = ptr->h_addr_list;
	while(*ptr3){
		memcpy(&addr, *ptr3, ptr->h_length);
		len += sprintf(&buf[len], "%s\n", inet_ntoa(addr));
		++ptr3;
	}
}

int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];
	int len;

	while(1){
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while(1){
			// ������ �ޱ�(���� ����)
			retval = recvn(client_sock, (char *)&len, sizeof(int), 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;

			// ������ �ޱ�(���� ����)
			retval = recvn(client_sock, buf, len, 0);
			if(retval == SOCKET_ERROR){
				err_display("recv()");
				break;
			}
			else if(retval == 0)
				break;

			if(!strncmp(buf, "exit", 4)) {
				closesocket(client_sock);
				closesocket(listen_sock);
				WSACleanup();
				return 0;
			}

			// ���� ������ ���
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
				ntohs(clientaddr.sin_port), buf);

			set_domain_info(buf);
			len = strlen(buf);

			// ������ ������(���� ����)
			retval = send(client_sock, (char *)&len, sizeof(int), 0);
			if(retval == SOCKET_ERROR){
				err_display("send()");
				break;
			}

			// ������ ������(���� ����)
			retval = send(client_sock, buf, len, 0);
			if(retval == SOCKET_ERROR){
				err_display("send()");
				break;
			}
		}

		// closesocket()
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	return 0;
}