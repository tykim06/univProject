#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "chat_cmd.h"
#include "host_info.h"

#define BUFSIZE     512
#define CHAR_BUFSIZE  30

host_info_t host_info;

bool is_overlap_mode;
char overlap_list[50][CHAR_BUFSIZE];
char overlap_list_count;

static void sys_cmd_set_host_ip(char *name, char *ip);
static void sys_cmd_overlap_mode(char *name, char *resv);
static const sys_cmd_t sys_cmd[] = {
	{":enter", sys_cmd_set_host_ip},
	{":overlap", sys_cmd_overlap_mode},
	{"",0}
};

static void user_cmd_print_help(void);
static void user_cmd_set_nick_name(void);
static void user_cmd_exit(void);
static const user_cmd_t user_cmd[] = {
	{":help", user_cmd_print_help, "show command list"},
	{":setNickName", user_cmd_set_nick_name, "set nick name"},
	{":exit", user_cmd_exit, "exit application"},
	{"",0,""}
};

// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

typedef int(*sock_send_fn_type)(char *);
typedef int(*sock_recv_fn_type)(char *, int *);
typedef struct Chat_Sock {
	SOCKET send_sock;
	SOCKET recv_sock;
	SOCKADDR_IN send_addr;
	SOCKADDR_IN recv_addr;
	sock_send_fn_type send;
	sock_recv_fn_type recv;
	struct ip_mreq mreq;
} chat_sock_t;
static int chat_sock_send(char *buf);
static int chat_sock_recv(char *buf, int length);
static chat_sock_t chat_sock;

static int chat_sock_send(char *buf) {
	int retval;
	char *name = host_info_get_p_name();
	time_t *send_time = host_info_get_p_time();
	
	retval = sendto(chat_sock.send_sock, name, strlen(name), 0,
			(SOCKADDR *)&chat_sock.send_addr, sizeof(chat_sock.send_addr));
	if(retval == SOCKET_ERROR) err_display("sendto()");
	
	char temp_buf[BUFSIZ+1];
	if(buf[0] == ':')  strcpy(temp_buf, buf);
	else sprintf(temp_buf, "%s\t||%s", buf, ctime(send_time));

	retval = sendto(chat_sock.send_sock, temp_buf, strlen(temp_buf), 0,
			(SOCKADDR *)&chat_sock.send_addr, sizeof(chat_sock.send_addr));
	if(retval == SOCKET_ERROR) err_display("sendto()");
	return retval;
}
static int chat_sock_recv(char *buf, int *length) {
	int retval = recvfrom(chat_sock.recv_sock, buf, BUFSIZE, 0, 
		(SOCKADDR *)&chat_sock.recv_addr, length);
	if(retval == SOCKET_ERROR) err_display("recvfrom()");
		
	return retval;
}

static void sys_cmd_set_host_ip(char *name, char *ip) {
	host_info_set_host_ip(name, ip);
}

static void sys_cmd_overlap_mode(char *name, char *resv) {
	if(strcmp(host_info_get_p_name(), name) == 0) {
		is_overlap_mode = true;
		printf("sys_cmd_overlap_mode1\n");
	} else  {
		strcpy(&overlap_list[overlap_list_count++][0], name);
		printf("sys_cmd_overlap_mode2\n");
	}
}

static void user_cmd_print_help(void) {
	unsigned int i=0;
	printf("\n");
	while(user_cmd[i].execute) {
		printf("%s => %s\n", user_cmd[i].cmd, user_cmd[i].desc);
		i++;
	}
	printf("\n");
}

static void user_cmd_set_nick_name(void) {
	char temp_nick_name[CHAR_BUFSIZE];
	printf("Enter new name : ");
	while(fgets(temp_nick_name, CHAR_BUFSIZE, stdin) == NULL);
	temp_nick_name[strlen(temp_nick_name)-1] = '\0';

	char temp_buf[100];
	sprintf(temp_buf, "NickName Update[%s > %s]", host_info_get_p_name, temp_nick_name);
	host_info_set_name(temp_nick_name);

	chat_sock.send(temp_buf);
}

static void user_cmd_exit(void) {
	// 멀티캐스트 그룹 탈퇴
	setsockopt(chat_sock.recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&chat_sock.mreq, sizeof(chat_sock.mreq));
	// closesocket()
	closesocket(chat_sock.recv_sock);
	closesocket(chat_sock.send_sock);
	// 윈속 종료
	WSACleanup();

	exit(0);
}

// 클라이언트와 데이터 통신
DWORD WINAPI Receiver(LPVOID arg)
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	chat_sock.recv_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(chat_sock.recv_sock == INVALID_SOCKET) err_quit("socket()");	

	// SO_REUSEADDR 옵션 설정
	BOOL optval = TRUE;
	retval = setsockopt(chat_sock.recv_sock, SOL_SOCKET,
		SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// bind()
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(host_info_get_multicast_port());
	retval = bind(chat_sock.recv_sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");
	
	// 멀티캐스트 그룹 가입
	chat_sock.mreq.imr_multiaddr.s_addr = inet_addr(host_info_get_p_multicast_ip());
	chat_sock.mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(chat_sock.recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&chat_sock.mreq, sizeof(chat_sock.mreq));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 데이터 통신에 사용할 변수
	int addrlen = sizeof(chat_sock.recv_addr);
	char buf[BUFSIZE+1];
	char name[CHAR_BUFSIZE];
	char *recv_ip_addr;

	// 멀티캐스트 데이터 받기
	while(1){				
		retval = chat_sock.recv(name, &addrlen);
		if(retval == SOCKET_ERROR) continue;
		name[retval] = '\0';

		retval = chat_sock.recv(buf, &addrlen);
		if(retval == SOCKET_ERROR) continue;
		buf[retval] = '\0';

		recv_ip_addr = inet_ntoa(chat_sock.recv_addr.sin_addr);

		if(buf[0] == ':') {
			parse_sys_cmd(buf, name, recv_ip_addr);
		} else if(host_info_is_overlapped_name(name, recv_ip_addr)) {
			sprintf(buf, ":overlap %s", host_info_get_p_name());
			chat_sock.send(buf);
		} else {
			printf("[%s][%s] %s\n", name, recv_ip_addr, buf);
		}
	}

	return 0;
}

void overlap_init() {
	is_overlap_mode = false;
	overlap_list_count = 0;
}

int main(int argc, char *argv[])
{
	host_info_init(&host_info);
	chat_cmd_init(sys_cmd, user_cmd);
	overlap_init();

	int retval;
	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	chat_sock.send_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(chat_sock.send_sock== INVALID_SOCKET) err_quit("socket()");

	// 멀티캐스트 TTL 설정
	int ttl = 2;
	retval = setsockopt(chat_sock.send_sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if(retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 소켓 주소 구조체 초기화
	ZeroMemory(&chat_sock.send_addr, sizeof(chat_sock.send_addr));
	chat_sock.send_addr.sin_family = AF_INET;
	chat_sock.send_addr.sin_addr.s_addr = inet_addr(host_info_get_p_multicast_ip());
	chat_sock.send_addr.sin_port = htons(host_info_get_multicast_port());

	chat_sock.send = chat_sock_send;
	chat_sock.recv = chat_sock_recv;

	// 데이터 통신에 사용할 변수
	char sendbuf[BUFSIZE+1];
	HANDLE hThread;

	//리시버 스레드 생성
	hThread = CreateThread(NULL, 0, Receiver,
	(LPVOID)chat_sock.send_sock, 0, NULL);
	if(hThread == NULL) { closesocket(chat_sock.send_sock); }
	else { CloseHandle(hThread); }

	Sleep(300);
	sprintf(sendbuf, ":enter");
	while(chat_sock.send(sendbuf) == SOCKET_ERROR);

	// 멀티캐스트 데이터 보내기
	while(1){
		// 데이터 입력
		while(fgets(sendbuf, BUFSIZE+1, stdin) == NULL);
		sendbuf[strlen(sendbuf)-1] = '\0';

		// 유저 명령어 확인
		if(sendbuf[0] == ':') {
			parse_user_cmd(sendbuf);
			continue;
		}

		// 데이터 보내기
		if(chat_sock.send(sendbuf) == SOCKET_ERROR) continue;
	}
	return 0;
}