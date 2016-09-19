#include "AMF_Win.h"
#include "process.h"

AMF_Win* AMF_Win::thisWin = NULL;

AMF_Win::AMF_Win(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow) {
	// MessageBox(NULL, "Constructing an AMF_Win", "Notice", MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND);

	if (thisWin) throw AMF_Win::CreationError();
	else thisWin = this;

	this->hInst = hInst;
	hWnd = NULL;
	SetFontToDefault();
	SetWindowDefault();
}

AMF_Win::~AMF_Win() {
	// MessageBox(NULL, "Destroying the AMF_Win", "Notice", MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND);
	thisWin = NULL;
};

void AMF_Win::CreateMainWindow() {
	RegisterClass(&winClass);
	hWnd = CreateWindow(
		winClass.lpszClassName,
		winName,
		winStyle,
		winPos.x, winPos.y,
		winSize.cx, winSize.cy,
		GetDesktopWindow(),
		0,
		hInst,
		NULL
	);
	return;
}

void AMF_Win::CreateSubWindows() {
}

LRESULT CALLBACK AMF_Win::MsgProcStatic(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {
	if (nMessage == WM_DESTROY) PostQuitMessage(NULL);
	if (thisWin)	return thisWin->MsgProc(hWnd, nMessage, wParam, lParam);
					return DefWindowProc(hWnd, nMessage, wParam, lParam);
}
LRESULT CALLBACK AMF_Win::MsgProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hWnd, nMessage, wParam, lParam);
};

void AMF_Win::SetWindowDefault() {

	winName = "AMF_Win";
	winPos.x = winPos.y = 100;
	winSize.cx = winSize.cy = 500;
	winStyle = WS_OVERLAPPEDWINDOW;

	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
	winClass.hInstance = hInst;
	winClass.hIcon = LoadIcon(hInst,IDI_APPLICATION);
	winClass.hCursor = LoadCursor(NULL,IDC_ARROW);
	winClass.lpfnWndProc = (WNDPROC) MsgProcStatic;
	winClass.lpszClassName = "MainWindow";
	winClass.lpszMenuName = NULL;
	winClass.style = CS_HREDRAW | CS_VREDRAW;
	return;
}

void AMF_Win::SetFontToDefault() {
	LOGFONT lf = {0};
	GetObject(hFont, sizeof(LOGFONT), &lf);
	strcpy(lf.lfFaceName,"Microsoft Sans Serif");
	lf.lfWeight = 100;
	lf.lfHeight = 14;
	lf.lfWidth = 5;
	hFont = CreateFontIndirect(&lf);
	return;
}

void AMF_Win::Run() {
	// MessageBox(NULL, "Running the AMF_Win", "Notice", MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND);
	// return;

	SetUp();
	CreateMainWindow();
	ShowWindow(hWnd, SW_SHOW);
	CreateSubWindows();

	MSG msg = {0};
	while(msg.message != WM_QUIT) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		OnRun();
	}
}

void AMF_Win::SetUp() {
	return;
}

void AMF_Win::OnRun() {
	return;
}