#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"

#define SERVERIPV4  "127.0.0.1"
#define SERVERIPV6  "::1"
#define SERVERPORT  9000

#define BUFSIZE     256                    // ���� �޽��� ��ü ũ��
#define MSGSIZE     (BUFSIZE-40*sizeof(char)-sizeof(int))  // ä�� �޽��� �ִ� ����

#define ENTER		1000
#define CH1			1001
#define CH2			1002

// ä�� �޽��� ����
// sizeof(CHAT_MSG) == 256
struct CHAT_MSG
{
	int  type;
	char nick[20];
	char dm_nick[20];
	char buf[MSGSIZE];
};

static HINSTANCE     g_hInst; // ���� ���α׷� �ν��Ͻ� �ڵ�
static HWND          g_hButtonSendMsg; // '�޽��� ����' ��ư
static HWND          g_hButtonShowList; // '������ ����Ʈ' ��ư
static HWND          g_hButtonIsDirect; // '��� ��ȭ' ��ư
static HWND          g_hEditStatus; // ���� �޽��� ���
static HWND          g_hEditStatus_DM; // ���� DM �޽��� ���
static char          g_ipaddr[64]; // ���� IP �ּ�
static u_short       g_port; // ���� ��Ʈ ��ȣ
static HANDLE        g_hClientThread; // ������ �ڵ�
static volatile BOOL g_bStart; // ��� ���� ����
static SOCKET        g_sock; // Ŭ���̾�Ʈ ����
static HANDLE        g_hReadEvent, g_hWriteEvent; // �̺�Ʈ �ڵ�
static CHAT_MSG      g_chatmsg; // ä�� �޽��� ����

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI ReadThread(LPVOID arg);
DWORD WINAPI WriteThread(LPVOID arg);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(int ch, char *fmt, ...);
// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags);
// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);

// ���� �Լ�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

	// �̺�Ʈ ����
	g_hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(g_hReadEvent == NULL) return 1;
	g_hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(g_hWriteEvent == NULL) return 1;

	// ���� �ʱ�ȭ(�Ϻ�)
	g_chatmsg.type = CH1;
	memset(g_chatmsg.nick, 0, sizeof(g_chatmsg.nick));
	memset(g_chatmsg.dm_nick, 0, sizeof(g_chatmsg.dm_nick));

	// ��ȭ���� ����
	g_hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// �̺�Ʈ ����
	CloseHandle(g_hReadEvent);
	CloseHandle(g_hWriteEvent);

	// ���� ����
	WSACleanup();
	return 0;
}

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

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButtonIsDM;
	static HWND hEditIPaddr;
	static HWND hEditPort;
	static HWND hButtonConnect;
	static HWND hEditMsg;
	static HWND hEditNickname;
	static HWND hEditDMNickname;
	static HWND hCH1;
	static HWND hCH2;

	switch(uMsg){
	case WM_INITDIALOG:
		// ��Ʈ�� �ڵ� ���
		hButtonIsDM = GetDlgItem(hDlg, IDC_CHECK_DM);
		hEditIPaddr = GetDlgItem(hDlg, IDC_IPADDR);
		hEditPort = GetDlgItem(hDlg, IDC_PORT);
		hButtonConnect = GetDlgItem(hDlg, IDC_CONNECT2);
		g_hButtonSendMsg = GetDlgItem(hDlg, IDC_SENDMSG);
		g_hButtonShowList = GetDlgItem(hDlg, IDC_SHOW_LIST);
		g_hButtonIsDirect = GetDlgItem(hDlg, IDC_CHECK_DM);
		hEditMsg = GetDlgItem(hDlg, IDC_MSG);
		hEditNickname = GetDlgItem(hDlg, IDC_NICKNAME);
		hEditDMNickname = GetDlgItem(hDlg, IDC_DM_NICKNAME);
		g_hEditStatus = GetDlgItem(hDlg, IDC_STATUS);
		g_hEditStatus_DM = GetDlgItem(hDlg, IDC_STATUS_DM);
		hCH1 = GetDlgItem(hDlg, IDC_CH1);
		hCH2 = GetDlgItem(hDlg, IDC_CH2);

		// ��Ʈ�� �ʱ�ȭ
		SendMessage(hEditMsg, EM_SETLIMITTEXT, MSGSIZE, 0);
		SendMessage(hEditNickname, EM_SETLIMITTEXT, 20, 0);
		SendMessage(hEditDMNickname, EM_SETLIMITTEXT, 20, 0);
		EnableWindow(g_hButtonSendMsg, FALSE);
		EnableWindow(g_hButtonShowList, FALSE);
		EnableWindow(g_hButtonIsDirect, FALSE);
		SetDlgItemText(hDlg, IDC_IPADDR, SERVERIPV4);
		SetDlgItemInt(hDlg, IDC_PORT, SERVERPORT, FALSE);
		SendMessage(hCH1, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hCH2, BM_SETCHECK, BST_UNCHECKED, 0);
		
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_CHECK_DM:
			if (SendMessage(hButtonIsDM, BM_GETCHECK, 0, 0)) {
				char nickname[20];
				GetDlgItemText(hDlg, IDC_DM_NICKNAME, nickname, sizeof(nickname));
				// todo : ���� �г��� ��ȿ�� �˻�
				strcpy(g_chatmsg.dm_nick, nickname);
			}
			else {
				memset(g_chatmsg.dm_nick, 0, sizeof(g_chatmsg.dm_nick));
			}
			return TRUE;

		case IDC_CONNECT2:
			char ipaddr[64];
			GetDlgItemText(hDlg, IDC_IPADDR, ipaddr, sizeof(g_ipaddr));
			if (!is_valid_ip(ipaddr)) {
				MessageBox(hDlg, "�ùٸ��� ���� IP �ּ��Դϴ�.", "����!", MB_ICONERROR);
				return TRUE;
			}
			strcpy(g_ipaddr, ipaddr);

			char c_port[10];
			unsigned short port;
			GetDlgItemText(hDlg, IDC_PORT, c_port, sizeof(c_port));
			if (!is_valid_port(c_port, &port)) {
				MessageBox(hDlg, "�ùٸ��� ���� PORT �Դϴ�.", "����!", MB_ICONERROR);
				return TRUE;
			}
			g_port = port;

			char nickname[20];
			GetDlgItemText(hDlg, IDC_NICKNAME, nickname, sizeof(nickname));
			if (strlen(nickname) == 0) {
				MessageBox(hDlg, "�г����� �Է����ּ���.", "����!", MB_ICONERROR);
				return TRUE;
			}
			strcpy(g_chatmsg.nick, nickname);

			// ���� ��� ������ ����
			g_hClientThread = CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
			if(g_hClientThread == NULL){
				MessageBox(hDlg, "Ŭ���̾�Ʈ�� ������ �� �����ϴ�."
					"\r\n���α׷��� �����մϴ�.", "����!", MB_ICONERROR);
				EndDialog(hDlg, 0);
			}
			else{
				EnableWindow(hButtonConnect, FALSE);
				while(g_bStart == FALSE); // ���� ���� ���� ��ٸ�

				//todo : �г��� �ߺ� üũ

				EnableWindow(hEditIPaddr, FALSE);
				EnableWindow(hEditPort, FALSE);
				EnableWindow(g_hButtonSendMsg, TRUE);
				EnableWindow(g_hButtonShowList, TRUE);
				EnableWindow(g_hButtonIsDirect, TRUE);
				SetFocus(hEditMsg);
			}
			return TRUE;

		case IDC_SENDMSG:
			// �б� �ϷḦ ��ٸ�
			WaitForSingleObject(g_hReadEvent, INFINITE);
			GetDlgItemText(hDlg, IDC_MSG, g_chatmsg.buf, MSGSIZE);
			// ���� �ϷḦ �˸�
			SetEvent(g_hWriteEvent);
			// �Էµ� �ؽ�Ʈ ��ü�� ���� ǥ��
			SendMessage(hEditMsg, EM_SETSEL, 0, -1);
			return TRUE;

		case IDC_CH1:
			g_chatmsg.type = CH1;
			return TRUE;

		case IDC_CH2:
			g_chatmsg.type = CH2;
			return TRUE;

		case IDCANCEL:
			if(MessageBox(hDlg, "������ �����Ͻðڽ��ϱ�?",
				"����", MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				closesocket(g_sock);
				EndDialog(hDlg, IDCANCEL);
			}
			return TRUE;

		}
		return FALSE;
	}

	return FALSE;
}

bool isOverlappedNickname() {
	int retval;
	CHAT_MSG chat_msg;
	// ������ ������
	retval = send(g_sock, (char *)&g_chatmsg, BUFSIZE, 0);

	while (1) {
		retval = recvn(g_sock, (char *)&chat_msg, BUFSIZE, 0);
		if (retval == 0 || retval == SOCKET_ERROR) {
			break;
		}

		if (strlen(chat_msg.dm_nick) != 0) {
			if (strcmp(g_chatmsg.nick, chat_msg.dm_nick) != 0) continue;
		}
		if (g_chatmsg.type != chat_msg.type) continue;

		DisplayText(chat_msg.type, "[���� �޽���] %s\r\n", chat_msg.buf);
	}
	return false;
}

// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID arg)
{
	int retval;

	// socket()
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(g_ipaddr);
	serveraddr.sin_port = htons(g_port);
	retval = connect(g_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	g_bStart = TRUE;

	if (isOverlappedNickname()) {
		MessageBox(NULL, "�ߺ��� �г��� �Դϴ�.", "����!", MB_ICONERROR);
		closesocket(g_sock);
		return 0;
	}

	// �б� & ���� ������ ����
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	if(hThread[0] == NULL || hThread[1] == NULL){
		MessageBox(NULL, "�����带 ������ �� �����ϴ�."
			"\r\n���α׷��� �����մϴ�.",
			"����!", MB_ICONERROR);
		exit(1);
	}

	MessageBox(NULL, "������ �����߽��ϴ�.", "����!", MB_ICONINFORMATION);

	// ������ ���� ���
	retval = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	retval -= WAIT_OBJECT_0;
	if(retval == 0)
		TerminateThread(hThread[1], 1);
	else
		TerminateThread(hThread[0], 1);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	g_bStart = FALSE;

	MessageBox(NULL, "������ ������ �������ϴ�", "�˸�", MB_ICONINFORMATION);
	EnableWindow(g_hButtonSendMsg, FALSE);

	closesocket(g_sock);
	return 0;
}

// ������ �ޱ�
DWORD WINAPI ReadThread(LPVOID arg)
{
	int retval;
	CHAT_MSG chat_msg;

	while(1){
		retval = recvn(g_sock, (char *)&chat_msg, BUFSIZE, 0);
		if(retval == 0 || retval == SOCKET_ERROR){
			break;
		}

		if (strlen(chat_msg.dm_nick) != 0) {
			if (strcmp(g_chatmsg.nick, chat_msg.dm_nick) != 0) continue;
		}
		if (g_chatmsg.type != chat_msg.type) continue;

		DisplayText(chat_msg.type, "[���� �޽���] %s\r\n", chat_msg.buf);
	}

	return 0;
}

// ������ ������
DWORD WINAPI WriteThread(LPVOID arg)
{
	int retval;

	// ������ ������ ���
	while(1){
		// ���� �Ϸ� ��ٸ���
		WaitForSingleObject(g_hWriteEvent, INFINITE);

		// ���ڿ� ���̰� 0�̸� ������ ����
		if(strlen(g_chatmsg.buf) == 0){
			// '�޽��� ����' ��ư Ȱ��ȭ
			EnableWindow(g_hButtonSendMsg, TRUE);
			// �б� �Ϸ� �˸���
			SetEvent(g_hReadEvent);
			continue;
		}

		// ������ ������
		retval = send(g_sock, (char *)&g_chatmsg, BUFSIZE, 0);
		if(retval == SOCKET_ERROR){
			break;
		}

		// '�޽��� ����' ��ư Ȱ��ȭ
		EnableWindow(g_hButtonSendMsg, TRUE);
		// �б� �Ϸ� �˸���
		SetEvent(g_hReadEvent);
	}

	return 0;
}

// ����Ʈ ��Ʈ�ѿ� ���ڿ� ���
void DisplayText(int ch, char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[1024];
	vsprintf(cbuf, fmt, arg);

	int nLength;
	if (ch == CH1) {
		nLength = GetWindowTextLength(g_hEditStatus);
		SendMessage(g_hEditStatus, EM_SETSEL, nLength, nLength);
		SendMessage(g_hEditStatus, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
	}
	else {
		nLength = GetWindowTextLength(g_hEditStatus_DM);
		SendMessage(g_hEditStatus_DM, EM_SETSEL, nLength, nLength);
		SendMessage(g_hEditStatus_DM, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
	}

	va_end(arg);
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