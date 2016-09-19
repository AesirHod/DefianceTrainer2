#include "AMF_Win.h"
#include "process.h"

#define btnActivate 1

class HashPrinter : public AMF_Win {
public:
						HashPrinter(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow);
	virtual				~HashPrinter();
protected:
	LRESULT CALLBACK	MsgProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void				SetUp();
	void				OnRun();
	void				CreateSubWindows();
	void				InjectDll();
	void				InjectFunction(CProcess* PRC);
	void				ActivatePrinter();
	bool				hasKeyboard;
	HWND				buttonActivate;
	int					version;
	DWORD				allocatedMemory,	Ex_fopen,			Ex_fclose,		Ex_fprintf,
						Ex_strchr,			Ex_LOAD_HashName;
	CProcess*			SR1;
	CProcess*			SR2;
	CProcess*			Defiance;
};

HashPrinter::HashPrinter(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow) :
AMF_Win(hInst, hPrev, pCmdLine, nCmdShow) {
	buttonActivate = NULL;
	SR1 = SR2 = Defiance = NULL;
	version = 0;
	allocatedMemory = NULL;
	hasKeyboard = false;
	Run();
}

HashPrinter::~HashPrinter() {
}

void HashPrinter::SetUp() {
	winSize.cx = 300;
	winSize.cy = 200;
	this->winName = "The Nupraptor Device";
	return;
}

void HashPrinter::OnRun() {
	return;
}

void HashPrinter::CreateSubWindows() {
	buttonActivate = CreateWindowEx(
			0, "Button", "Begin Output", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			50, 20, 100, 20, this->hWnd, (HMENU)btnActivate, this->hInst, 0
	);
	SendDlgItemMessage(this->hWnd, btnActivate, WM_SETFONT, (WPARAM)hFont, FALSE);
	return;
}

void HashPrinter::ActivatePrinter() {
	if (!SR1) SR1 = new CProcess("Legacy of Kain: Soul Reaver");
	if (!SR2) SR2 = new CProcess("Soul Reaver 2");
	if (!Defiance) Defiance = new CProcess("Legacy of Kain: Defiance");
	char DllPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, DllPath);
	if ((DllPath[strlen(DllPath)-1]!='\\') &&
		(DllPath[strlen(DllPath)-1]!='/' ))
		strcat(DllPath, "\\\0");
	strcat(DllPath, "StdIOExtern.dll\0");

	if (SR1->FindProcess()) {
		if (SR1->WasUpdated()) {
			if		(SR1->CmpString(0x004cea18, "Sep 13 1999")) version = -1;
			else if	(SR1->CmpString(0x004d0a68, "Aug 30 1999"))	version = 1;
			else if (SR1->CmpString(0x004f1b50, "Oct 29 1999"))	version = 2;
			else												version = 0;
			allocatedMemory = NULL;
			if (version==-1) {
				Ex_strchr			= 0x004bf300;
				Ex_LOAD_HashName	= 0x0046c760;
			}
			else if (version==1) {
				Ex_strchr			= 0x004c0a60;
				Ex_LOAD_HashName	= 0x0046cd80;
			}
			else if (version==2) {
				Ex_strchr			= 0x004dc9e0;
				Ex_LOAD_HashName	= 0x0047ac10;
			}
		}
		if (version!=0 && !allocatedMemory) {
			DWORD dllLocation = NULL;
			dllLocation = SR1->InjectDll(DllPath);
			Ex_fopen	= dllLocation + 0x000010BD;	// DEADFACE
			Ex_fclose	= dllLocation + 0x0000111C;	// BEEFC0DE
			Ex_fprintf	= dllLocation + 0x0000116D;	// C0DE5ACE
			InjectFunction(SR1);
			SR1->WriteDiff(Ex_LOAD_HashName+0x32, allocatedMemory+0x10);
		}
	}
	else if (SR2->FindProcess()) {
		if (SR2->WasUpdated()) {
			if		(SR2->CmpString(0x004E6854, "Nov  2 2001"))	version = -1;
			else if	(SR2->CmpString(0x004ED9A4, "Jan  4 2002"))	version = 1;
			else if (SR2->CmpString(0x004E9854, "Dec 11 2001"))	version = 2;
			else												version = 0;
			allocatedMemory = NULL;
			if (version==-1) {
				Ex_strchr = 0x004d62a0;
				Ex_LOAD_HashName = 0x004B56F0;
			}
			else if (version==1) {
				Ex_strchr = 0x004db660;
				Ex_LOAD_HashName = 0x004BB3B0;
			}
			else if (version==2) {
				Ex_strchr = 0x004D8FE0;
				Ex_LOAD_HashName = 0x004B7FD0;
			}
		}
		if (version!=0 && !allocatedMemory) {
			DWORD dllLocation = NULL;
			dllLocation = SR2->InjectDll(DllPath);
			Ex_fopen	= dllLocation + 0x000010BD;	// DEADFACE
			Ex_fclose	= dllLocation + 0x0000111C;	// BEEFC0DE
			Ex_fprintf	= dllLocation + 0x0000116D; // C0DE5ACE
			InjectFunction(SR2);
			SR2->WriteDiff(Ex_LOAD_HashName+0x28, allocatedMemory+0x10);
		}
	}
	else if (Defiance->FindProcess()) {
		if (Defiance->WasUpdated()) {
			/*if		(SR2->CmpString(0x004E6854, "Nov  2 2001"))	version = -1;
			else if	(SR2->CmpString(0x004ED9A4, "Jan  4 2002"))	version = 1;
			else if (SR2->CmpString(0x004E9854, "Dec 11 2001"))	version = 2;
			else												version = 0;*/
			version = 2;
			allocatedMemory = NULL;
			if (version==2) {
				Ex_strchr = 0x0053a992;			// Not really strchr
				Ex_LOAD_HashName = 0x00446030;	// Not really LOAD_HashName
			}
		}
		if (version!=0 && !allocatedMemory) {
			DWORD dllLocation = NULL;
			dllLocation = Defiance->InjectDll(DllPath);
			Ex_fopen	= dllLocation + 0x000010BD;	// DEADFACE
			Ex_fclose	= dllLocation + 0x0000111C;	// BEEFC0DE
			Ex_fprintf	= dllLocation + 0x0000116D; // C0DE5ACE
			InjectFunction(Defiance);
			Defiance->WriteDiff(Ex_LOAD_HashName+0x1F, allocatedMemory+0x10);
		}
	}
	return;
}

void HashPrinter::InjectFunction(CProcess* PRC) {

	DWORD mem = allocatedMemory = PRC->Alloc(1024);
	PRC->WriteString(	mem+=PRC->SizeOfLast(), "output.txt\0");
	PRC->WriteString(	mem+=PRC->SizeOfLast(), "at\0");
	PRC->WriteString(	mem+=PRC->SizeOfLast(), " \0");
	if (PRC==Defiance)
		PRC->WriteXString(	mem+=PRC->SizeOfLast(), "60 89 c5 68");	// Try fd instead of c5
	else
		PRC->WriteXString(	mem+=PRC->SizeOfLast(), "60 89 d5 68");
	PRC->Write(			mem+=PRC->SizeOfLast(),	(DWORD)(allocatedMemory+0x0B));
	PRC->Write(			mem+=PRC->SizeOfLast(),	(BYTE)0x68);
	PRC->Write(			mem+=PRC->SizeOfLast(),	(DWORD)allocatedMemory);
	PRC->Write(			mem+=PRC->SizeOfLast(),	(BYTE)0xE8);
	PRC->WriteDiff(		mem+=PRC->SizeOfLast(),	Ex_fopen);
	PRC->WriteXString(	mem+=PRC->SizeOfLast(), "83 c4 08 55 50 e8");
	PRC->WriteDiff(		mem+=PRC->SizeOfLast(),	Ex_fprintf);
	PRC->WriteXString(	mem+=PRC->SizeOfLast(), "58 5d 68");
	PRC->Write(			mem+=PRC->SizeOfLast(),	(DWORD)(allocatedMemory+0x0E));
	PRC->WriteXString(	mem+=PRC->SizeOfLast(),	"50 e8");
	PRC->WriteDiff(		mem+=PRC->SizeOfLast(),	Ex_fprintf);
	PRC->WriteXString(	mem+=PRC->SizeOfLast(), "58 83 c4 04 50 e8");
	PRC->WriteDiff(		mem+=PRC->SizeOfLast(),	Ex_fclose);
	PRC->WriteXString(	mem+=PRC->SizeOfLast(),	"83 c4 04 61 e9");
	PRC->WriteDiff(		mem+=PRC->SizeOfLast(), Ex_strchr);
	PRC->WriteXString(	mem+=PRC->SizeOfLast(), "90 90 90 90");
	return;
}

LRESULT CALLBACK HashPrinter::MsgProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {
	switch(nMessage) {
		case WM_COMMAND:
			switch(HIWORD(wParam))
				case BN_CLICKED:
					if ((LOWORD(wParam))==btnActivate) ActivatePrinter();
					break;
				case LBN_DBLCLK:
					break;
				case EN_CHANGE:
					break;
			break;
	}
	return DefWindowProc(hWnd, nMessage, wParam, lParam);
};

int	WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow) {
	HashPrinter* printer = new HashPrinter(hInst, hPrev, pCmdLine, nCmdShow);
	delete printer;
	printer = NULL;
	return NULL;
}