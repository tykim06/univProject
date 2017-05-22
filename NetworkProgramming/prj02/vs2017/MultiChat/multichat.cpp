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

#define EDIT_BUFSIZE 25
#define BUFSIZE     512
#define CHAR_BUFSIZE  30

host_info_t host_info;

bool is_enter_mode;
bool is_overlap_mode;

static char recv_name[CHAR_BUFSIZE];
static char *recv_ip_addr;

// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID arg);

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...);

HWND hEdit1, hEdit2; // ���� ��Ʈ��

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// ���� ��� ������ ����
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	// ��ȭ���� ����
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return 0;
}

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static char buf[EDIT_BUFSIZE + 1];

	switch (uMsg) {
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			DisplayText("%s\r\n", buf);
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

// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE + 256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
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
	if (retval == SOCKET_ERROR) err_display("sendto()");

	char temp_buf[BUFSIZ + 1];
	if (buf[0] == ':')  strcpy(temp_buf, buf);
	else sprintf(temp_buf, "%s\t||%s", buf, ctime(send_time));

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
		printf("\n\nChange Name by Entering :setName\n\n");
		is_overlap_mode = true;
	}
}

static void sys_cmd_argu_name(char *buf) {
	if (host_info_is_equal_name(recv_name)) {
		char *msg = &strstr(buf, " ")[1];
		printf("[%s][%s] %s\n", recv_name, recv_ip_addr, msg);
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
	printf("\n");
	while (user_cmd[i].execute) {
		printf("%s => %s\n", user_cmd[i].cmd, user_cmd[i].desc);
		i++;
	}
	printf("\n");
}

static void user_cmd_set_name(void) {
	char temp_name[CHAR_BUFSIZE];
	printf("Enter new name : ");
	while (fgets(temp_name, CHAR_BUFSIZE, stdin) == NULL);
	temp_name[strlen(temp_name) - 1] = '\0';

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
	// ��Ƽĳ��Ʈ �׷� Ż��
	setsockopt(chat_sock.recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
		(char *)&chat_sock.mreq, sizeof(chat_sock.mreq));
	// closesocket()
	closesocket(chat_sock.recv_sock);
	closesocket(chat_sock.send_sock);
	// ���� ����
	WSACleanup();

	exit(0);
}

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI Receiver(LPVOID arg)
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	chat_sock.recv_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (chat_sock.recv_sock == INVALID_SOCKET) err_quit("socket()");

	// SO_REUSEADDR �ɼ� ����
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

	// ��Ƽĳ��Ʈ �׷� ����
	chat_sock.mreq.imr_multiaddr.s_addr = inet_addr(host_info_get_p_multicast_ip());
	chat_sock.mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(chat_sock.recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char *)&chat_sock.mreq, sizeof(chat_sock.mreq));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// ������ ��ſ� ����� ����
	int addrlen = sizeof(chat_sock.recv_addr);
	char buf[BUFSIZE + 1];

	// ��Ƽĳ��Ʈ ������ �ޱ�
	while (1) {
		retval = chat_sock.recv(recv_name, &addrlen);
		if (retval == SOCKET_ERROR) continue;
		recv_name[retval] = '\0';

		retval = chat_sock.recv(buf, &addrlen);
		if (retval == SOCKET_ERROR) continue;
		buf[retval] = '\0';

		recv_ip_addr = inet_ntoa(chat_sock.recv_addr.sin_addr);

		if (buf[0] == ':') {
			parse_sys_cmd(buf);
		}
		else if (!is_overlap_mode) {
			printf("[%s][%s] %s\n", recv_name, recv_ip_addr, buf);
		}
	}

	return 0;
}

void overlap_init() {
	is_enter_mode = false;
	is_overlap_mode = false;
}

DWORD WINAPI ClientMain(LPVOID arg)
{
	host_info_init(&host_info);
	chat_cmd_init(sys_cmd, user_cmd);
	overlap_init();

	int retval;
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	chat_sock.send_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (chat_sock.send_sock == INVALID_SOCKET) err_quit("socket()");

	// ��Ƽĳ��Ʈ TTL ����
	int ttl = 2;
	retval = setsockopt(chat_sock.send_sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(char *)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	// ���� �ּ� ����ü �ʱ�ȭ
	ZeroMemory(&chat_sock.send_addr, sizeof(chat_sock.send_addr));
	chat_sock.send_addr.sin_family = AF_INET;
	chat_sock.send_addr.sin_addr.s_addr = inet_addr(host_info_get_p_multicast_ip());
	chat_sock.send_addr.sin_port = htons(host_info_get_multicast_port());

	chat_sock.send = chat_sock_send;
	chat_sock.recv = chat_sock_recv;

	// ������ ��ſ� ����� ����
	char sendbuf[BUFSIZE + 1];
	HANDLE hThread;

	//���ù� ������ ����
	hThread = CreateThread(NULL, 0, Receiver,
		(LPVOID)chat_sock.send_sock, 0, NULL);
	if (hThread == NULL) { closesocket(chat_sock.send_sock); }
	else { CloseHandle(hThread); }

	Sleep(300);
	sprintf(sendbuf, ":enter");
	while (chat_sock.send(sendbuf) == SOCKET_ERROR);

	// ��Ƽĳ��Ʈ ������ ������
	while (1) {
		// ������ �Է�
		while (fgets(sendbuf, BUFSIZE + 1, stdin) == NULL);
		sendbuf[strlen(sendbuf) - 1] = '\0';

		// ���� ��ɾ� Ȯ��
		if (sendbuf[0] == ':') {
			parse_user_cmd(sendbuf);
			continue;
		}

		if (is_overlap_mode) {
			char temp_buf[BUFSIZE + 1];
			strcpy(temp_buf, sendbuf);
			sprintf(sendbuf, ":argu %s", temp_buf);
		}
		while (chat_sock.send(sendbuf) == SOCKET_ERROR);
	}
	return 0;
}