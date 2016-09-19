// Based on code by Jared Bruni http://www.lostsidedead.com/gameprog
// Additional functions and adaptions added by Vampmaster

#ifndef INC_PROCESS_H
#define INC_PROCESS_H

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

extern void findMe();

class CProcess 
{
public:
	CProcess();
	CProcess			(char *p_WindowTitle);
	virtual ~CProcess	();
	BOOL	IsRunning	();
	BOOL	FindProcess	(char *p_WindowTitle);
	BOOL	FindProcess	();
	BOOL	Reset		();
	BOOL	WasUpdated	();
	BOOL	IsKeyPressed(int p_iKey);
	BOOL	IsSpyRunning(char *p_WindowTitle);
	DWORD	Alloc		(int bytes);
	DWORD	Free		(DWORD address);
	DWORD	InjectDll	(char* dllName);
	template<class TData>
	BOOL	Write		(DWORD p_Address, TData data);
	template<class TData>
	BOOL	Write		(DWORD p_Address, TData* data, DWORD size);
	BYTE*	WriteXString(DWORD p_Address, char* hexStr);
	DWORD	WriteString	(DWORD p_Address, char* ascStr);
	template<class TData>
	TData	Read		(TData* p_Address);
	template<class TData>
	TData*	Read		(TData* p_Address, DWORD size);
	template<class TData>
	BOOL	CmpData		(DWORD address, TData data);
	char*	CmpString	(DWORD address, char* ascStr);
	DWORD	AddrDiff	(DWORD origin, DWORD dest);
	BOOL	WriteDiff	(DWORD origin, DWORD dest);
	DWORD	SizeOfLast	();

private:
	HANDLE OpenProcess(char *p_ClassName, char *p_WindowTitle);
	HWND hWindow;
	HWND pWindow;
	BOOL updated;
	HANDLE m_hProcess;
	BOOL m_bGameRunning;
	char *windowTitle;
	DWORD bytesWritten;
};

#endif