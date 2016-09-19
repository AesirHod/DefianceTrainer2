#pragma once
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>

class AMF_Win {
public:
						AMF_Win(HINSTANCE hInst, HINSTANCE hPrev, LPSTR pCmdLine, int nCmdShow);
	virtual				~AMF_Win();
	struct				CreationError {};
protected:
	virtual LRESULT CALLBACK
						MsgProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	virtual void		SetUp();
	virtual void		OnRun();
	void				Run();
	void				SetWindowDefault();
	void				SetFontToDefault();
	virtual void		CreateSubWindows();
	char*				winName;
	WNDCLASS			winClass;
	HINSTANCE			hInst;
	HFONT				hFont;
	HWND				hWnd;
	POINT				winPos;
	SIZE				winSize;
	DWORD				winStyle;
private:
	static AMF_Win*		thisWin;
	static LRESULT CALLBACK
						MsgProcStatic(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void				CreateMainWindow();
	AMF_Win(const AMF_Win&);
	AMF_Win& operator=(const AMF_Win&);
};