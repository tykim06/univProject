#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#include "DataValidator.h"

#define SERVERIPV4  "127.0.0.1"
#define SERVERIPV6  "::1"
#define SERVERPORT  9000

#define BUFSIZE     256                    // ���� �޽��� ��ü ũ��
#define MSGSIZE     (BUFSIZE-48*sizeof(char)-sizeof(int))  // ä�� �޽��� �ִ� ����

#define ENTER		1000
#define CH1			1001
#define CH2			1002
#define LIST		1003
#define DM			1004

// ä�� �޽��� ����
// sizeof(CHAT_MSG) == 256
struct CHAT_MSG
{
	int  type;
	char nick[24];
	char dm_nick[24];
	char buf[MSGSIZE];
};

typedef enum {
	OVERLAP_STATUS_NO_OVERLAPPED,
	OVERLAP_STATUS_OVERLAPPED,
	OVERLAP_STATUS_NULL
} overlap_status_t;

typedef enum {
	DM_STATUS_NO_MATCH,
	DM_STATUS_MATCH,
	DM_STATUS_NULL
} dm_status_t;

static volatile BOOL g_eNickname; // ��� ���� ����
static volatile BOOL g_eDM; // ��� ���� ����
static HINSTANCE     g_hInst; // ���� ���α׷� �ν��Ͻ� �ڵ�
static HWND          g_hButtonSendMsg; // '�޽��� ����' ��ư
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
void DisplayText(BOOL isDirect, char *fmt, ...);
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
	g_chatmsg.type = ENTER;
	memset(g_chatmsg.nick, 0, sizeof(g_chatmsg.nick));
	memset(g_chatmsg.dm_nick, 0, sizeof(g_chatmsg.dm_nick));
	g_eNickname = OVERLAP_STATUS_NULL;
	g_eDM = DM_STATUS_NULL;

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

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButtonIsDM;
	static HWND hEditIPaddr;
	static HWND hEditPort;
	static HWND hButtonConnect;
	static HWND hButtonOverlap;
	static HWND hButtonShowList;
	static HWND hEditMsg;
	static HWND hEditNickname;
	static HWND hEditDMNickname;
	static HWND hCH1;
	static HWND hCH2;

	CHAT_MSG chatMsg;

	switch(uMsg){
	case WM_INITDIALOG:
		// ��Ʈ�� �ڵ� ���
		hButtonIsDM = GetDlgItem(hDlg, IDC_CHECK_DM);
		hEditIPaddr = GetDlgItem(hDlg, IDC_IPADDR);
		hEditPort = GetDlgItem(hDlg, IDC_PORT);
		hButtonConnect = GetDlgItem(hDlg, IDC_CONNECT2);
		hButtonOverlap = GetDlgItem(hDlg, IDC_CHECK_OVERLAP);
		hButtonShowList = GetDlgItem(hDlg, IDC_SHOW_LIST);
		g_hButtonSendMsg = GetDlgItem(hDlg, IDC_SENDMSG);
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
		EnableWindow(g_hButtonSendMsg, FALSE);

		SendMessage(hEditNickname, EM_SETLIMITTEXT, 20, 0);
		EnableWindow(hButtonShowList, FALSE);
		EnableWindow(hButtonOverlap, FALSE);

		SendMessage(hEditDMNickname, EM_SETLIMITTEXT, 20, 0);
		EnableWindow(g_hButtonIsDirect, FALSE);

		SetDlgItemText(hDlg, IDC_IPADDR, SERVERIPV4);
		SetDlgItemInt(hDlg, IDC_PORT, SERVERPORT, FALSE);

		SendMessage(hCH1, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(hCH2, BM_SETCHECK, BST_UNCHECKED, 0);
		EnableWindow(hCH1, FALSE);
		EnableWindow(hCH2, FALSE);
		
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_CHECK_DM:
			if (!IsDlgButtonChecked(hDlg, IDC_CHECK_DM)) {
				memset(g_chatmsg.dm_nick, 0, 24);
				return TRUE;
			}
			GetDlgItemText(hDlg, IDC_DM_NICKNAME, chatMsg.dm_nick, sizeof(chatMsg.dm_nick));
			if (!is_valid_nickname(chatMsg.dm_nick)) {
				MessageBox(hDlg, "�ùٸ� ���� �г����� �Է����ּ���.", "����!", MB_ICONERROR);
				CheckDlgButton(hDlg, IDC_CHECK_DM, FALSE);
				return TRUE;
			}

			chatMsg.type = DM;
			// ������ ������
			if (send(g_sock, (char *)&chatMsg, BUFSIZE, 0) == SOCKET_ERROR) {
				MessageBox(hDlg, "�ٽ� �õ����ּ���.", "����!", MB_ICONERROR);
				CheckDlgButton(hDlg, IDC_CHECK_DM, FALSE);
				break;
			}

			while (g_eDM == DM_STATUS_NULL);

			if (g_eDM == DM_STATUS_NO_MATCH) {
				MessageBox(hDlg, "�������� �ʴ� �г��� �Դϴ�.", "����!", MB_ICONERROR);
				CheckDlgButton(hDlg, IDC_CHECK_DM, FALSE);
			}
			else {
				MessageBox(hDlg, "��� ��ȭ�� ���۵˴ϴ�.", "����!", MB_ICONINFORMATION);
			}
			g_eDM = DM_STATUS_NULL;
			strcpy(g_chatmsg.dm_nick, chatMsg.dm_nick);

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

				EnableWindow(hEditIPaddr, FALSE);
				EnableWindow(hEditPort, FALSE);
				EnableWindow(hButtonOverlap, TRUE);
				SetFocus(hEditNickname);
			}
			return TRUE;
		
		case IDC_CHECK_OVERLAP:
			GetDlgItemText(hDlg, IDC_NICKNAME, chatMsg.nick, sizeof(chatMsg.nick));
			if (!is_valid_nickname(chatMsg.nick)) {
				MessageBox(hDlg, "�ùٸ��� ���� �г��� �Դϴ�.", "����!", MB_ICONERROR);
				return TRUE;
			}
			
			strcpy(g_chatmsg.nick, chatMsg.nick);
			EnableWindow(hButtonOverlap, FALSE);
			// ������ ������
			if (send(g_sock, (char *)&g_chatmsg, BUFSIZE, 0) == SOCKET_ERROR) {
				MessageBox(hDlg, "�ٽ� �õ����ּ���.", "����!", MB_ICONERROR);
				break;
			}
			while (g_eNickname == OVERLAP_STATUS_NULL); // �ߺ� �˻� ��� �� ��ٸ�

			if (g_eNickname == OVERLAP_STATUS_OVERLAPPED) {
				MessageBox(hDlg, "��� �Ұ����� �г��� �Դϴ�.", "����!", MB_ICONERROR);
				EnableWindow(hButtonOverlap, TRUE);
				SetFocus(hEditNickname);
				return TRUE;
			}
			else {
				MessageBox(hDlg, "��� ������ �г��� �Դϴ�.", "�˸�", MB_ICONINFORMATION);
			}

			EnableWindow(g_hButtonSendMsg, TRUE);
			EnableWindow(hButtonShowList, TRUE);
			EnableWindow(g_hButtonIsDirect, TRUE);
			EnableWindow(hCH1, TRUE);
			EnableWindow(hCH2, TRUE);
			SetFocus(hEditMsg);
			g_chatmsg.type = (IsDlgButtonChecked(hDlg, IDC_CH1)) ? CH1 : CH2;
			break;

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
			if (g_chatmsg.type == CH1) return TRUE;
			g_chatmsg.type = CH1;
			if (MessageBox(hDlg, "�г����� �����Ͻðڽ��ϱ�?",
				"����", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				g_chatmsg.type = ENTER;
				EnableWindow(hButtonOverlap, TRUE);
				EnableWindow(g_hButtonSendMsg, FALSE);
				EnableWindow(hButtonShowList, FALSE);
				EnableWindow(g_hButtonIsDirect, FALSE);
				EnableWindow(hCH1, FALSE);
				EnableWindow(hCH2, FALSE);
			}
			return TRUE;

		case IDC_CH2:
			if (g_chatmsg.type == CH2) return TRUE;
			g_chatmsg.type = CH2;
			if (MessageBox(hDlg, "�г����� �����Ͻðڽ��ϱ�?",
				"����", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				g_chatmsg.type = ENTER;
				EnableWindow(hButtonOverlap, TRUE);
				EnableWindow(g_hButtonSendMsg, FALSE);
				EnableWindow(hButtonShowList, FALSE);
				EnableWindow(g_hButtonIsDirect, FALSE);
				EnableWindow(hCH1, FALSE);
				EnableWindow(hCH2, FALSE);
			}
			return TRUE;

		case IDCANCEL:
			if(MessageBox(hDlg, "������ �����Ͻðڽ��ϱ�?",
				"����", MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				closesocket(g_sock);
				EndDialog(hDlg, IDCANCEL);
			}
			return TRUE;
		case IDC_SHOW_LIST:
			chatMsg.type = LIST;
			// ������ ������
			if (send(g_sock, (char *)&chatMsg, BUFSIZE, 0) == SOCKET_ERROR) {
				MessageBox(hDlg, "�ٽ� �õ����ּ���.", "����!", MB_ICONERROR);
				break;
			}
			DisplayText(FALSE, "\r\n\r\n\t�������� ���� ���\r\n");
			break;

		}
		return FALSE;
	}

	return FALSE;
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

	MessageBox(NULL, "������ �����߽��ϴ�.", "����!", MB_ICONINFORMATION);

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

	g_bStart = TRUE;

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

	MessageBox(NULL, "������ ������ �������ϴ�.", "�˸�", MB_ICONINFORMATION);
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

		if (chat_msg.type == ENTER) {
			if (strlen(chat_msg.nick) != 0) {
				g_eNickname = OVERLAP_STATUS_NO_OVERLAPPED;
			}
			else {
				g_eNickname = OVERLAP_STATUS_OVERLAPPED;
			}
			continue;
		}

		if (chat_msg.type == LIST) {
			DisplayText(FALSE, "%s\r\n", chat_msg.buf);
			continue;
		}

		if (chat_msg.type == DM) {
			if (strlen(chat_msg.dm_nick) != 0) {
				g_eDM = DM_STATUS_MATCH;
			}
			else {
				g_eDM = DM_STATUS_NO_MATCH;
			}
			continue;
		}

		if (strlen(chat_msg.dm_nick) != 0) {
			if (strcmp(g_chatmsg.nick, chat_msg.dm_nick) == 0
				|| strcmp(g_chatmsg.nick, chat_msg.nick) == 0)
				DisplayText(TRUE, "[%s]==> %s\r\n", chat_msg.nick, chat_msg.buf);
			continue;
		}
		if (g_chatmsg.type != chat_msg.type) continue;

		DisplayText(FALSE, "[%s]==> %s\r\n", chat_msg.nick, chat_msg.buf);
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
void DisplayText(BOOL isDirect, char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char cbuf[1024];
	vsprintf(cbuf, fmt, arg);

	int nLength;
	if (!isDirect) {
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