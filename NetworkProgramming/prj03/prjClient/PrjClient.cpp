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

#define BUFSIZE     256                    // 전송 메시지 전체 크기
#define MSGSIZE     (BUFSIZE-48*sizeof(char)-sizeof(int))  // 채팅 메시지 최대 길이

#define ENTER		1000
#define CH1			1001
#define CH2			1002
#define LIST		1003
#define DM			1004

// 채팅 메시지 형식
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

static volatile BOOL g_eNickname; // 통신 시작 여부
static volatile BOOL g_eDM; // 통신 시작 여부
static HINSTANCE     g_hInst; // 응용 프로그램 인스턴스 핸들
static HWND          g_hButtonSendMsg; // '메시지 전송' 버튼
static HWND          g_hButtonIsDirect; // '비밀 대화' 버튼
static HWND          g_hEditStatus; // 받은 메시지 출력
static HWND          g_hEditStatus_DM; // 받은 DM 메시지 출력
static char          g_ipaddr[64]; // 서버 IP 주소
static u_short       g_port; // 서버 포트 번호
static HANDLE        g_hClientThread; // 스레드 핸들
static volatile BOOL g_bStart; // 통신 시작 여부
static SOCKET        g_sock; // 클라이언트 소켓
static HANDLE        g_hReadEvent, g_hWriteEvent; // 이벤트 핸들
static CHAT_MSG      g_chatmsg; // 채팅 메시지 저장

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);
DWORD WINAPI ReadThread(LPVOID arg);
DWORD WINAPI WriteThread(LPVOID arg);
// 편집 컨트롤 출력 함수
void DisplayText(BOOL isDirect, char *fmt, ...);
// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags);
// 오류 출력 함수
void err_quit(char *msg);
void err_display(char *msg);

// 메인 함수
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0) return 1;

	// 이벤트 생성
	g_hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if(g_hReadEvent == NULL) return 1;
	g_hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(g_hWriteEvent == NULL) return 1;

	// 변수 초기화(일부)
	g_chatmsg.type = ENTER;
	memset(g_chatmsg.nick, 0, sizeof(g_chatmsg.nick));
	memset(g_chatmsg.dm_nick, 0, sizeof(g_chatmsg.dm_nick));
	g_eNickname = OVERLAP_STATUS_NULL;
	g_eDM = DM_STATUS_NULL;

	// 대화상자 생성
	g_hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

	// 이벤트 제거
	CloseHandle(g_hReadEvent);
	CloseHandle(g_hWriteEvent);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 대화상자 프로시저
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
		// 컨트롤 핸들 얻기
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

		// 컨트롤 초기화
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
				MessageBox(hDlg, "올바른 상대방 닉네임을 입력해주세요.", "실패!", MB_ICONERROR);
				CheckDlgButton(hDlg, IDC_CHECK_DM, FALSE);
				return TRUE;
			}

			chatMsg.type = DM;
			// 데이터 보내기
			if (send(g_sock, (char *)&chatMsg, BUFSIZE, 0) == SOCKET_ERROR) {
				MessageBox(hDlg, "다시 시도해주세요.", "실패!", MB_ICONERROR);
				CheckDlgButton(hDlg, IDC_CHECK_DM, FALSE);
				break;
			}

			while (g_eDM == DM_STATUS_NULL);

			if (g_eDM == DM_STATUS_NO_MATCH) {
				MessageBox(hDlg, "존재하지 않는 닉네임 입니다.", "실패!", MB_ICONERROR);
				CheckDlgButton(hDlg, IDC_CHECK_DM, FALSE);
			}
			else {
				MessageBox(hDlg, "비밀 대화가 시작됩니다.", "성공!", MB_ICONINFORMATION);
			}
			g_eDM = DM_STATUS_NULL;
			strcpy(g_chatmsg.dm_nick, chatMsg.dm_nick);

			return TRUE;

		case IDC_CONNECT2:
			char ipaddr[64];
			GetDlgItemText(hDlg, IDC_IPADDR, ipaddr, sizeof(g_ipaddr));
			if (!is_valid_ip(ipaddr)) {
				MessageBox(hDlg, "올바르지 않은 IP 주소입니다.", "실패!", MB_ICONERROR);
				return TRUE;
			}
			strcpy(g_ipaddr, ipaddr);

			char c_port[10];
			unsigned short port;
			GetDlgItemText(hDlg, IDC_PORT, c_port, sizeof(c_port));
			if (!is_valid_port(c_port, &port)) {
				MessageBox(hDlg, "올바르지 않은 PORT 입니다.", "실패!", MB_ICONERROR);
				return TRUE;
			}
			g_port = port;

			// 소켓 통신 스레드 시작
			g_hClientThread = CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
			if(g_hClientThread == NULL){
				MessageBox(hDlg, "클라이언트를 시작할 수 없습니다."
					"\r\n프로그램을 종료합니다.", "실패!", MB_ICONERROR);
				EndDialog(hDlg, 0);
			}
			else{
				EnableWindow(hButtonConnect, FALSE);
				while(g_bStart == FALSE); // 서버 접속 성공 기다림

				EnableWindow(hEditIPaddr, FALSE);
				EnableWindow(hEditPort, FALSE);
				EnableWindow(hButtonOverlap, TRUE);
				SetFocus(hEditNickname);
			}
			return TRUE;
		
		case IDC_CHECK_OVERLAP:
			GetDlgItemText(hDlg, IDC_NICKNAME, chatMsg.nick, sizeof(chatMsg.nick));
			if (!is_valid_nickname(chatMsg.nick)) {
				MessageBox(hDlg, "올바르지 않은 닉네임 입니다.", "실패!", MB_ICONERROR);
				return TRUE;
			}
			
			strcpy(g_chatmsg.nick, chatMsg.nick);
			EnableWindow(hButtonOverlap, FALSE);
			// 데이터 보내기
			if (send(g_sock, (char *)&g_chatmsg, BUFSIZE, 0) == SOCKET_ERROR) {
				MessageBox(hDlg, "다시 시도해주세요.", "실패!", MB_ICONERROR);
				break;
			}
			while (g_eNickname == OVERLAP_STATUS_NULL); // 중복 검사 결과 값 기다림

			if (g_eNickname == OVERLAP_STATUS_OVERLAPPED) {
				MessageBox(hDlg, "사용 불가능한 닉네임 입니다.", "실패!", MB_ICONERROR);
				EnableWindow(hButtonOverlap, TRUE);
				SetFocus(hEditNickname);
				return TRUE;
			}
			else {
				MessageBox(hDlg, "사용 가능한 닉네임 입니다.", "알림", MB_ICONINFORMATION);
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
			// 읽기 완료를 기다림
			WaitForSingleObject(g_hReadEvent, INFINITE);
			GetDlgItemText(hDlg, IDC_MSG, g_chatmsg.buf, MSGSIZE);
			// 쓰기 완료를 알림
			SetEvent(g_hWriteEvent);
			// 입력된 텍스트 전체를 선택 표시
			SendMessage(hEditMsg, EM_SETSEL, 0, -1);
			return TRUE;

		case IDC_CH1:
			if (g_chatmsg.type == CH1) return TRUE;
			g_chatmsg.type = CH1;
			if (MessageBox(hDlg, "닉네임을 변경하시겠습니까?",
				"질문", MB_YESNO | MB_ICONQUESTION) == IDYES)
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
			if (MessageBox(hDlg, "닉네임을 변경하시겠습니까?",
				"질문", MB_YESNO | MB_ICONQUESTION) == IDYES)
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
			if(MessageBox(hDlg, "정말로 종료하시겠습니까?",
				"질문", MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				closesocket(g_sock);
				EndDialog(hDlg, IDCANCEL);
			}
			return TRUE;
		case IDC_SHOW_LIST:
			chatMsg.type = LIST;
			// 데이터 보내기
			if (send(g_sock, (char *)&chatMsg, BUFSIZE, 0) == SOCKET_ERROR) {
				MessageBox(hDlg, "다시 시도해주세요.", "실패!", MB_ICONERROR);
				break;
			}
			DisplayText(FALSE, "\r\n\r\n\t접속중인 유저 목록\r\n");
			break;

		}
		return FALSE;
	}

	return FALSE;
}

// 소켓 통신 스레드 함수
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

	MessageBox(NULL, "서버에 접속했습니다.", "성공!", MB_ICONINFORMATION);

	// 읽기 & 쓰기 스레드 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	if(hThread[0] == NULL || hThread[1] == NULL){
		MessageBox(NULL, "스레드를 시작할 수 없습니다."
			"\r\n프로그램을 종료합니다.",
			"실패!", MB_ICONERROR);
		exit(1);
	}

	g_bStart = TRUE;

	// 스레드 종료 대기
	retval = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	retval -= WAIT_OBJECT_0;
	if(retval == 0)
		TerminateThread(hThread[1], 1);
	else
		TerminateThread(hThread[0], 1);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);

	g_bStart = FALSE;

	MessageBox(NULL, "서버가 접속을 끊었습니다.", "알림", MB_ICONINFORMATION);
	EnableWindow(g_hButtonSendMsg, FALSE);

	closesocket(g_sock);
	return 0;
}

// 데이터 받기
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

// 데이터 보내기
DWORD WINAPI WriteThread(LPVOID arg)
{
	int retval;

	// 서버와 데이터 통신
	while(1){
		// 쓰기 완료 기다리기
		WaitForSingleObject(g_hWriteEvent, INFINITE);

		// 문자열 길이가 0이면 보내지 않음
		if(strlen(g_chatmsg.buf) == 0){
			// '메시지 전송' 버튼 활성화
			EnableWindow(g_hButtonSendMsg, TRUE);
			// 읽기 완료 알리기
			SetEvent(g_hReadEvent);
			continue;
		}

		// 데이터 보내기
		retval = send(g_sock, (char *)&g_chatmsg, BUFSIZE, 0);
		if(retval == SOCKET_ERROR){
			break;
		}

		// '메시지 전송' 버튼 활성화
		EnableWindow(g_hButtonSendMsg, TRUE);
		// 읽기 완료 알리기
		SetEvent(g_hReadEvent);
	}

	return 0;
}

// 에디트 컨트롤에 문자열 출력
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

// 사용자 정의 데이터 수신 함수
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