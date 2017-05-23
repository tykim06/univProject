#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <time.h>
#include "chat_cmd.h"
#include "host_info.h"
#include <windows.h>
#include <stdio.h>
#include "resource.h"

#define BUFSIZE     512
#define CHAR_BUFSIZE  30

bool is_enter_mode;
bool is_overlap_mode;

static char recv_name[CHAR_BUFSIZE];
static char *recv_ip_addr;

char send_buf[BUFSIZE + 1];
char recv_buf[BUFSIZE + 1];

HWND hEdit1, hEdit2; // 편집 컨트롤
HANDLE hWriteEvent; // 이벤트

// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE + 1];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
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
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
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
static int chat_sock_recv(char *buf, int *length);
static chat_sock_t chat_sock;

static int chat_sock_send(char *buf) {
	int retval;
	char *name = host_info_get_p_name();

	retval = sendto(chat_sock.send_sock, name, strlen(name), 0,
		(SOCKADDR *)&chat_sock.send_addr, sizeof(chat_sock.send_addr));
	if (retval == SOCKET_ERROR) err_display("sendto()");

	char temp_buf[BUFSIZ + 1];
	if (buf[0] == ':')  strcpy(temp_buf, buf);
	else sprintf(temp_buf, "%s\t|| %s", buf, host_info_get_current_time());

	retval = sendto(chat_sock.send_sock, temp_buf, strlen(temp_buf), 0,
		(SOCKADDR *)&chat_sock.send_addr, sizeof(chat_sock.send_addr));
	if (retval == SOCKET_ERROR) err_display("sendto()");
	return retval;
}
static int chat_sock_recv(char *buf, int *length) {
	int retval = recvfrom(chat_sock.recv_sock, buf, BUFSIZE, 0,
		(SOCKADDR *)&chat_sock.recv_addr, length);
	if (retval == SOCKET_ERROR) err_display("recvfrom()");

	return retval;
}

static void sys_cmd_enter_user(char *buf);
static void sys_cmd_overlap_user(char *buf);
static void sys_cmd_argu_name(char *buf);
static void sys_cmd_rename(char *buf);
static const sys_cmd_t sys_cmd[] = {
	{ ":enter", sys_cmd_enter_user },
	{ ":overlap", sys_cmd_overlap_user },
	{ ":argu", sys_cmd_argu_name },
	{ ":rename", sys_cmd_rename },
	{ "",0 }
};

static void sys_cmd_enter_user(char *buf) {
	if (host_info_is_equal_name(recv_name)) {
		if (!is_enter_mode) {
			is_enter_mode = true;
		}
		else {
			chat_sock.send(":overlap");
		}
	}
}

static void sys_cmd_overlap_user(char *buf) {
	if (host_info_is_equal_name(recv_name)) {
		DisplayText("\n\nChange Name by Entering :setName\n\n");
		is_overlap_mode = true;
	}
}

static void sys_cmd_argu_name(char *buf) {
	if (host_info_is_equal_name(recv_name)) {
		char *msg = &strstr(buf, " ")[1];
		DisplayText("[%s][%s] %s\n", recv_name, recv_ip_addr, msg);
	}
}

static void sys_cmd_rename(char *buf) {
	if (host_info_is_equal_name(recv_name)) {
		is_overlap_mode = false;
	}
}

static void user_cmd_print_help(void);
static void user_cmd_set_name(void);
static void user_cmd_exit(void);
static const user_cmd_t user_cmd[] = {
	{ ":help", user_cmd_print_help, "show command list" },
	{ ":setName", user_cmd_set_name, "set name" },
	{ ":exit", user_cmd_exit, "exit application" },
	{ "",0,"" }
};

static void user_cmd_print_help(void) {
	unsigned int i = 0;
	DisplayText("\n");
	while (user_cmd[i].execute) {
		DisplayText("%s => %s\n", user_cmd[i].cmd, user_cmd[i].desc);
		i++;
	}
	DisplayText("\n");
}

static void user_cmd_set_name(void) {
	char temp_name[CHAR_BUFSIZE];
	DisplayText("\t\t<<<< Enter new name >>>>\n");
	WaitForSingleObject(hWriteEvent, INFINITE);
	strcpy(temp_name, send_buf);

	is_enter_mode = false;

	if (is_overlap_mode) {
		is_overlap_mode = false;
		chat_sock.send(":rename");
	}

	char temp_buf[100];
	sprintf(temp_buf, "Name Update[ %s >>> %s ]", host_info_get_p_name(), temp_name);
	host_info_set_name(temp_name);
	chat_sock.send(temp_buf);
	chat_sock.send(":enter");
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
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	chat_sock.recv_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (chat_sock.recv_sock == INVALID_SOCKET) err_quit("socket()");

	// SO_REUSEADDR 옵션 설정
	BOOL optval = TRUE;
	retval = setsockopt(chat_sock.recv_sock, SOL_SOCKET,
		SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// bind()
	SOCKADDR_IN localaddr;
	ZeroMemory(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(host_info_get_multicast_port());
	retval = bind(chat_sock.recv_sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// 멀티캐스트 그룹 가입
	chat_sock.mreq.imr_multiaddr.s_addr = inet_addr(host_info_get_p_multicast_ip());
	chat_sock.mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(chat_sock.recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&chat_sock.mreq, sizeof(chat_sock.mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 데이터 통신에 사용할 변수
	int addrlen = sizeof(chat_sock.recv_addr);

	// 멀티캐스트 데이터 받기
	while (1) {
		retval = chat_sock.recv(recv_name, &addrlen);
		if (retval == SOCKET_ERROR) continue;
		recv_name[retval] = '\0';

		retval = chat_sock.recv(recv_buf, &addrlen);
		if (retval == SOCKET_ERROR) continue;
		recv_buf[retval] = '\0';

		recv_ip_addr = inet_ntoa(chat_sock.recv_addr.sin_addr);

		if (recv_buf[0] == ':') {
			parse_sys_cmd(recv_buf);
		}
		else if (!is_overlap_mode) {
			DisplayText("[%s][%s] %s\n", recv_name, recv_ip_addr, recv_buf);
		}
	}

	return 0;
}

bool is_class_d_ip(char *multi_ip) {
	long ip = inet_addr(multi_ip);
	if ((ip & 0xff) < 224 || (ip & 0xff) > 239) return false;
	ip = ip >> 8;
	if ((ip & 0xff) > 255 || (ip & 0xff) < 0) return false;
	ip = ip >> 8;
	if ((ip & 0xff) > 255 || (ip & 0xff) < 0) return false;
	ip = ip >> 8;
	if ((ip & 0xff) > 255 || (ip & 0xff) < 0) return false;

	return true;
}

static void chat_info_init() {
	char multi_cast_ip[CHAR_BUFSIZE] = { 0, };
	unsigned short multi_cast_port = 0;
	char name[CHAR_BUFSIZE] = { 0, };
	 
	do {
		WaitForSingleObject(hWriteEvent, INFINITE);
		strcpy(multi_cast_ip, send_buf);
		if (is_class_d_ip(multi_cast_ip)) break;
		DisplayText("Wrong IP\n");
	} while(1);
	do {
		DisplayText("\t\t<<<< Enter multicast port >>>>\n");
		WaitForSingleObject(hWriteEvent, INFINITE);
		if (sscanf(send_buf, "%hd", &multi_cast_port))
			if (multi_cast_port >= 0 || multi_cast_port <= 65535) break;
		DisplayText("Wrong Port\n");
	} while (1);
	DisplayText("\t\t<<<< Enter your nickname >>>>\n");
	WaitForSingleObject(hWriteEvent, INFINITE);
	strcpy(name, send_buf);

	DisplayText("start multicast chat! ip: %s, port: %d, name: %s\n", multi_cast_ip, multi_cast_port, name);
	DisplayText("More information Enter :help\n\n\n");

	host_info_init(multi_cast_ip, multi_cast_port, name);
}

void overlap_init() {
	is_enter_mode = false;
	is_overlap_mode = false;
}

DWORD WINAPI ClientMain(LPVOID arg)
{
	chat_info_init();
	chat_cmd_init(sys_cmd, user_cmd);
	overlap_init();

	int retval;
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	chat_sock.send_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (chat_sock.send_sock == INVALID_SOCKET) err_quit("socket()");

	// 멀티캐스트 TTL 설정
	int ttl = 2;
	retval = setsockopt(chat_sock.send_sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// 소켓 주소 구조체 초기화
	ZeroMemory(&chat_sock.send_addr, sizeof(chat_sock.send_addr));
	chat_sock.send_addr.sin_family = AF_INET;
	chat_sock.send_addr.sin_addr.s_addr = inet_addr(host_info_get_p_multicast_ip());
	chat_sock.send_addr.sin_port = htons(host_info_get_multicast_port());

	chat_sock.send = chat_sock_send;
	chat_sock.recv = chat_sock_recv;

	// 데이터 통신에 사용할 변수
	HANDLE hThread;

	//리시버 스레드 생성
	hThread = CreateThread(NULL, 0, Receiver,
		(LPVOID)chat_sock.send_sock, 0, NULL);
	if (hThread == NULL) { closesocket(chat_sock.send_sock); }
	else { CloseHandle(hThread); }

	Sleep(300);
	sprintf(send_buf, ":enter");
	while (chat_sock.send(send_buf) == SOCKET_ERROR);

	// 멀티캐스트 데이터 보내기
	while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 기다리기

		// 유저 명령어 확인
		if (send_buf[0] == ':') {
			if (!parse_user_cmd(send_buf)) DisplayText("Wrong Command\n");
			continue;
		}

		if (is_overlap_mode) {
			char temp_buf[BUFSIZE + 1];
			strcpy(temp_buf, send_buf);
			sprintf(send_buf, ":argu %s", temp_buf);
		}
		while (chat_sock.send(send_buf) == SOCKET_ERROR);
	}
	return 0;
}

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		DisplayText("\t\t<<<< Enter multicast ip >>>>\n");
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, send_buf, BUFSIZE + 1);
			SetEvent(hWriteEvent); // 쓰기 완료 알리기
			SetFocus(hEdit1);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// 이벤트 생성
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return 1;

	// 소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// 이벤트 제거
	CloseHandle(hWriteEvent);

	// 멀티캐스트 그룹 탈퇴
	setsockopt(chat_sock.recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&chat_sock.mreq, sizeof(chat_sock.mreq));

	// closesocket()
	closesocket(chat_sock.recv_sock);
	closesocket(chat_sock.send_sock);

	// 윈속 종료
	WSACleanup();

	return 0;
}