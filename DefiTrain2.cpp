#include "AMF_Win.h"
#include "process.h"
#include "resource.h"

#define ID_DebugOn			VK_F1
#define ID_DebugOff			VK_F2
#define ID_CamDefi			VK_F3
#define ID_CamSRvr			VK_F4
#define ID_CamFree			VK_F5
#define ID_About			1	// Button Only
#define camDefi				3
#define camSRvr				2
#define camFree				7
#define TIMER				1
#define TIMER_INTERRUPT		500	// Milliseconds

class DefiTrainer : public AMF_Win {
public:
						DefiTrainer(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow);
	virtual				~DefiTrainer();
protected:
	void				SetUp();
	void				OnRun();
	void				CreateSubWindows();
	LRESULT CALLBACK	MsgProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void				WriteMemory(WORD keyPressed);
	bool				SetUpProcess();
	void				DebugOn();
	void				DebugOff();
	void				CameraMode(WORD mode);
	void				AboutMsg();
	bool				hasKeyboard;
	HWND				btnDebugOn, btnDebugOff, btnAbout;
	BYTE				version;
	WORD				cameraMode;
	DWORD				allocatedMemory;
	DWORD				memTracker;
	bool				doneOneTimeStuff;
	CProcess*			Defiance;
};

DefiTrainer::DefiTrainer(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow) :
AMF_Win(hInst, hPrev, pCmdLine, nCmdShow) {
	// Buttons
	btnDebugOn			= NULL;
	btnDebugOff			= NULL;
	btnAbout			= NULL;
	// Connection with game
	Defiance			= new CProcess("Legacy of Kain: Defiance");
	version				= 0;
	cameraMode			= camDefi;
	// In game memory
	allocatedMemory		= NULL;
	memTracker			= NULL;
	// flags
	hasKeyboard			= false;
	doneOneTimeStuff	= false;
	Run();
}

DefiTrainer::~DefiTrainer() {
}

void DefiTrainer::OnRun() {
	return;
}

void DefiTrainer::SetUp() {
	winSize.cx = 320;
	winSize.cy = 166;
	winName = "Legacy of Kain: Defiance Trainer v2.1";
	winClass.hbrBackground = CreatePatternBrush(LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BITMAP1)));
	winClass.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ICON1));
	winClass.hCursor = LoadCursor(hInst,MAKEINTRESOURCE(IDC_CURSOR1));
	winStyle = WS_SYSMENU | WS_MINIMIZEBOX;
	return;
}

void DefiTrainer::CreateSubWindows() {
	/*btnDebugOn = CreateWindowEx(
			0, "Button", "Debug On", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			110, 40, 100, 20, hWnd, (HMENU)ID_DebugOn, hInst, 0
	);
	SendDlgItemMessage(hWnd, ID_DebugOn, WM_SETFONT, (WPARAM)hFont, FALSE);
	SetClassLong(btnDebugOn, GCL_HCURSOR, (long) winClass.hCursor);
	btnDebugOff = CreateWindowEx(
			0, "Button", "Debug Off", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			110, 65, 100, 20, hWnd, (HMENU)ID_DebugOff, hInst, 0
	);
	SendDlgItemMessage(hWnd, ID_DebugOff, WM_SETFONT, (WPARAM)hFont, FALSE);
	SetClassLong(btnDebugOff, GCL_HCURSOR, (long) winClass.hCursor);*/
	btnAbout = CreateWindowEx(
			0, "Button", "About", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			213, 1, 100, 20, hWnd, (HMENU)ID_About, hInst, 0
	);
	SendDlgItemMessage(hWnd, ID_About, WM_SETFONT, (WPARAM)hFont, FALSE);
	SetClassLong(btnAbout, GCL_HCURSOR, (long) winClass.hCursor);
	return;
}

LRESULT CALLBACK DefiTrainer::MsgProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {
	switch(nMessage) {
		case WM_CREATE:
			SetTimer(hWnd, TIMER, TIMER_INTERRUPT, NULL);
			break;
		case WM_DESTROY: {
			KillTimer(hWnd, TIMER);
			PostQuitMessage(0);
			break;
		}
		case WM_COMMAND:
			switch(HIWORD(wParam)) {
				case BN_CLICKED:
					if (LOWORD(wParam)==ID_About) AboutMsg();
					else WriteMemory(LOWORD(wParam));
					break;
				case LBN_DBLCLK:
					break;
				case EN_CHANGE:
					break;
			}
			break;
		case WM_TIMER: {
			WriteMemory(NULL);
			break;
		}
	}
	return DefWindowProc(hWnd, nMessage, wParam, lParam);
};

void DefiTrainer::WriteMemory(WORD keyPressed) {
	if (!keyPressed) {
		if (Defiance->IsKeyPressed(ID_DebugOn)) {
			keyPressed = ID_DebugOn;
		}
		if (Defiance->IsKeyPressed(ID_DebugOff)) {
			keyPressed = ID_DebugOff;
		}
		if (Defiance->IsKeyPressed(ID_CamDefi)) {
			keyPressed = ID_CamDefi;
		}
		if (Defiance->IsKeyPressed(ID_CamSRvr)) {
			keyPressed = ID_CamSRvr;
		}
		if (Defiance->IsKeyPressed(ID_CamFree)) {
			keyPressed = ID_CamFree;
		}
	}
	switch (keyPressed) {
		case ID_DebugOn:
			DebugOn();
			break;
		case ID_DebugOff:
			DebugOff();
			break;
		case ID_CamDefi:
			CameraMode(camDefi);
			break;
		case ID_CamSRvr:
			CameraMode(camSRvr);
			break;
		case ID_CamFree:
			CameraMode(camFree);
			break;
		default:
			//CameraMode(cameraMode);
			break;
	}
	return;
}

bool DefiTrainer::SetUpProcess() {
	Defiance->FindProcess();
	if (Defiance->WasUpdated()) {
		allocatedMemory = Defiance->Alloc(1024);
		memTracker = allocatedMemory;
		doneOneTimeStuff = false;
		cameraMode = camDefi;
	}
	if (Defiance->IsRunning()) return true;
	return false;
}

void DefiTrainer::DebugOn() {

	if (!SetUpProcess()) return;
	if (Defiance->CmpData(0x0053feea, (BYTE) 0x30)) {
		Defiance->WriteXString(0x0041d4fe, "75 2B");
		Defiance->WriteXString(0x0041d50b, "7D 1E");
		Defiance->WriteXString(0x0041d50d, "8B 04 C5 BC A3 54 00");
		if (!doneOneTimeStuff) {
			Defiance->WriteXString(0x0041c9f0, "69 C9 D8 01 00 00");
			Defiance->WriteXString(0x0041ca54, "69 C0 40 00 00 00");
			Defiance->WriteXString(0x0041cb24, "69 C0 40 00 00 00");
			Defiance->WriteXString(0x0041cf94, "69 C0 20 02 00 00");
			Defiance->WriteXString(0x0041cfa0, "B1 20");
			Defiance->WriteXString(0x0041cfd4, "69 C0 60 01 00 00");
			Defiance->WriteXString(0x0041cff4, "69 C0 60 01 00 00");
			// Overwrite Autoface menu with PlaneShift
			Defiance->WriteXString(0x00548e68, "06");
			Defiance->WriteXString(0x00548e74, "5d e8 54 00");
			Defiance->WriteXString(0x00548e78, "10 5d 50 00");
			// Add the Shiva Menu
			Defiance->Write			(0x00549318, (DWORD)0x00000007);
			Defiance->Write			(0x00549324, (DWORD)memTracker);
			Defiance->WriteString	(memTracker, "Shiva Menu...\0");
			memTracker +=			Defiance->SizeOfLast();
			Defiance->Write			(0x00549328, (DWORD)0x00549540);
			Defiance->Write			(0x00549330, (DWORD)0x0000000B);
			Defiance->Write			(0x0054933C, (DWORD)0x0053EE86);
			Defiance->Write			(0x00549340, (DWORD)0x00620B98);
			// Insert store room into menu
			Defiance->Write			(0x00548784, (DWORD)memTracker);
			Defiance->WriteString	(memTracker, "Store Room...\0");
			memTracker +=			Defiance->SizeOfLast();
			Defiance->Write			(0x00548788, (DWORD)0x00548250);
			Defiance->Write			(0x0054879c, (DWORD)0x0053EED8);
			Defiance->Write			(0x005487a0, (DWORD)0x00548268);
			Defiance->WriteString	(memTracker, "Artifact 1A\0");
			Defiance->Write			(0x00548274, (DWORD)memTracker);
			Defiance->Write			(0x0054826c, (DWORD)0x00000001);
			doneOneTimeStuff = true;
		}
		// menuState
		Defiance->WriteXString(0x00620c68, "04");
	}
	else if (Defiance->CmpData(0x0053feea, (BYTE)0xFF)) {
		if (!doneOneTimeStuff) {
			// Bind controls to menu
			Defiance->WriteXString(0x0041cde0, "69 C9 D8 01 00 00");
			Defiance->WriteXString(0x0041ce44, "69 C0 40 00 00 00");
			Defiance->WriteXString(0x0041cf14, "69 C0 40 00 00 00");
			Defiance->WriteXString(0x0041d384, "69 C0 20 02 00 00");
			Defiance->WriteXString(0x0041d390, "B1 20");
			Defiance->WriteXString(0x0041d3c4, "69 C0 60 01 00 00");
			Defiance->WriteXString(0x0041d3e4, "69 C0 60 01 00 00");
			// Overwrite Autoface menu with PlaneShift
			Defiance->Write			(0x0054be58, (DWORD)0x00000006);
			Defiance->Write			(0x0054be68, (DWORD)0x00509030);
			Defiance->Write			(0x0054be64, (DWORD)0x0055188D);
			// Add the Shiva Menu
			Defiance->Write			(0x0054C308, (DWORD)0x00000007);
			Defiance->Write			(0x0054C314, (DWORD)memTracker);
			Defiance->WriteString	(memTracker, "Shiva Menu...\0");
			memTracker +=			Defiance->SizeOfLast();
			Defiance->Write			(0x0054C318, (DWORD)0x0054C530);
			Defiance->Write			(0x0054C320, (DWORD)0x0000000B);
			Defiance->Write			(0x0054C32C, (DWORD)0x00541E86);
			Defiance->Write			(0x0054C330, (DWORD)0x00623DB0);
			// Insert store room into menu
			Defiance->Write			(0x0054b774, (DWORD)memTracker);
			Defiance->WriteString	(memTracker, "Store Room...\0");
			memTracker +=			Defiance->SizeOfLast();
			Defiance->Write			(0x0054b778, (DWORD)0x0054b240);
			Defiance->Write			(0x0054b78c, (DWORD)0x00541ec8);
			Defiance->Write			(0x0054b790, (DWORD)0x0054b1b0);
			Defiance->WriteString	(memTracker, "Artifact 1A\0");
			Defiance->Write			(0x0054b264, (DWORD)memTracker);
			Defiance->Write			(0x0054b25c, (DWORD)0x00000001);
			doneOneTimeStuff = true;
		}
		Defiance->WriteXString(0x0041d8ee, "75 2B");
		Defiance->WriteXString(0x0041d8fb, "7D 1E");
		Defiance->WriteXString(0x0041d8fd, "8B 04 C5 E4 D3 54 00");
		// menuState
		Defiance->WriteXString(0x00623e80, "04");
	}
	return;
}

void DefiTrainer::DebugOff() {

	if (!SetUpProcess()) return;
	if (Defiance->CmpData(0x0053feea, (BYTE) 0x30)) {
		Defiance->WriteXString(0x0041d4fe, "90 90");
		Defiance->WriteXString(0x0041d50b, "90 90");
		Defiance->WriteXString(0x0041d50d, "B8 60 A5 43 00 90 90");
	}
	else if (Defiance->CmpData(0x0053feea, (BYTE)0xFF)) {
		Defiance->WriteXString(0x0041d8ee, "90 90");
		Defiance->WriteXString(0x0041d8fb, "90 90");
		Defiance->WriteXString(0x0041d8fd, "B8 60 A9 43 00 90 90");
	}
	return;
}

void DefiTrainer::CameraMode(WORD mode) {
	cameraMode = mode;
	if (!SetUpProcess()) return;
	if (Defiance->CmpData(0x0053feea, (BYTE) 0x30)) {
		Defiance->Write(0x0056d7bc, mode);
	}
	else if (Defiance->CmpData(0x0053feea, (BYTE)0xFF)) {
		Defiance->Write(0x005709dc, mode);
	}
	return;
}

void DefiTrainer::AboutMsg() {
	char info[] =
"A device to enable the developers debug menu and\r\n\
other deleted features of Legacy of Kain: Defiance.\r\n\
Based on the PS2 codes by Ben Lincoln and CMX.\r\n\
Thanks to CubanRaul for helping with version 1.\r\n\
More additions to come!\r\n\
\r\n\
In game commands:\r\n\
\r\n\
F1:\tDebug Menu On\r\n\
F2:\tDebug Menu Off\r\n\
F3:\tDefiance Camera\r\n\
F4:\tSoul Reaver 1/2 Camera\r\n\
F5:\tFree Roaming Camera (Analog Stick Only!)\r\n\
\r\n\
Debug menu controls:\r\n\
\r\n\
TK Aim:\t\t\tUp\r\n\
Lift Attack:\t\tDown\r\n\
Reaver Select Left:\t\tDecrease Value\r\n\
Reaver Select Right:\tIncrease Value\r\n\
Jump:\t\t\tSelect\r\n\
\r\n\
Version 2.1\r\n\
Copyright 2006,\tAndrew Fradley";
	MessageBox(hWnd, info, "About", MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND);
	return;
}

int	WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow) {
	DefiTrainer* trainer = new DefiTrainer(hInst, hPrev, pCmdLine, nCmdShow);
	delete trainer;
	trainer = NULL;
	return NULL;
}