#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#define WM_EXITTHREAD (WM_APP+100)
#define IDC_DISP 1000
#define IDC_START 1001
#define IDC_STOP 1002

TCHAR szClassName[] = TEXT("Window");
BOOL b;

BOOL getprime(unsigned char *prime_table, DWORD index) {
	return (prime_table[index / 8] >> (index % 8)) & 1;
}

VOID setprime(unsigned char *prime_table, DWORD index) {
	prime_table[index / 8] |= 1 << (index % 8);
}

DWORD WINAPI ThreadFunc(LPVOID p)
{
	WPARAM wParam = 0;
	DWORD i, k, count = 0;
	unsigned char * prime_table = (unsigned char *)LocalAlloc(LPTR, 536870912);//0x100000000/0x8
	if (prime_table == 0) { SetDlgItemText((HWND)p, IDC_DISP, TEXT("メモリが足りません")); goto EXIT0; }
	SetDlgItemText((HWND)p, IDC_DISP, TEXT("素数表を作成しています..."));
	for (i = 2; i<65536; i++) {
		if (!b) { goto EXIT1; }
		if (!getprime(prime_table, i))
			for (k = i*i; k >= i*i; k += i)
				setprime(prime_table, k);
	}
	SetDlgItemText((HWND)p, IDC_DISP, TEXT("ファイルに出力しています..."));
	const HANDLE hFile = CreateFile(TEXT("prime32.txt"), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	for (i = 2; i; i++) {
		TCHAR szTemp[1024];
		DWORD d;
		if (!b) { goto EXIT2; }
		if (getprime(prime_table, i) == 0) {
			wsprintf(szTemp, TEXT("%u\r\n"), i);
			WriteFile(hFile, szTemp, lstrlen(szTemp), &d, 0);
		}
	}
	wParam = 1;
EXIT2:
	CloseHandle(hFile);
EXIT1:
	LocalFree(prime_table);
EXIT0:
	SetDlgItemText((HWND)p, IDC_DISP, b ? TEXT("完了しました。") : TEXT("中断しました。"));
	PostMessage((HWND)p, WM_EXITTHREAD, wParam, 0);
	ExitThread(1);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hThread;
	static HWND hStatic;
	static HWND hButtonStart;
	static HWND hButtonStop;
	switch (msg)
	{
	case WM_CREATE:
		hStatic = CreateWindow(TEXT("STATIC"), 0, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDC_DISP, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButtonStart = CreateWindow(TEXT("BUTTON"), TEXT("スタート"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDC_START, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButtonStop = CreateWindow(TEXT("BUTTON"), TEXT("ストップ"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDC_STOP, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hStatic, 10, 10, 256, 32, TRUE);
		MoveWindow(hButtonStart, 10, 50, 256, 32, TRUE);
		MoveWindow(hButtonStop, 10, 90, 256, 32, TRUE);
		break;
	case WM_EXITTHREAD:
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		hThread = 0;
		MessageBeep(wParam ? MB_ICONASTERISK : MB_OK);
		EnableWindow(hButtonStart, 1);
		EnableWindow(hButtonStop, 0);
		SetFocus(hButtonStart);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_START)
		{
			SetWindowText(hStatic, 0);
			EnableWindow(hButtonStart, 0);
			EnableWindow(hButtonStop, 1);
			SetFocus(hButtonStop);
			b = 1;
			DWORD dwParam;
			hThread = CreateThread(0, 0, ThreadFunc, (void*)hWnd, 0, &dwParam);
		}
		else if (LOWORD(wParam) == IDC_STOP)
		{
			SetWindowText(hStatic, TEXT("中断しています..."));
			b = 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("32bit素数表をファイルに書き出す"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
