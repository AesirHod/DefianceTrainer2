#include "process.h"

CProcess::CProcess()
{
	m_hProcess = NULL;
	m_bGameRunning = false;
	windowTitle = NULL;
	bytesWritten = 0;
}

CProcess::CProcess(char *p_WindowTitle)
{
	m_hProcess = NULL;
	m_bGameRunning = false;
	this->FindProcess(p_WindowTitle);
	bytesWritten = 0;
}

CProcess::~CProcess()
{
	if (m_bGameRunning) CloseHandle(m_hProcess);
}

BOOL CProcess::WasUpdated()
{
	BOOL ud = updated;
	updated = false;
	bytesWritten = 0;
	return ud;
}

BOOL CProcess::Reset()
{
	m_hProcess = NULL;
	m_bGameRunning = false;
	return true;
}

HANDLE CProcess::OpenProcess(char *p_ClassName, char *p_WindowTitle)
{
	DWORD CREATE_THREAD_ACCESS	=	PROCESS_CREATE_THREAD	|	PROCESS_QUERY_INFORMATION	|
									PROCESS_VM_OPERATION	|	PROCESS_VM_WRITE	|
									PROCESS_VM_READ			|	PROCESS_VM_OPERATION;
	DWORD pid; // Process ID
	hWindow = FindWindow(p_ClassName, p_WindowTitle);
    if (pWindow != hWindow) updated = true;
	pWindow = hWindow;
	if (hWindow)
	{
		GetWindowThreadProcessId(hWindow, &pid);
		return ::OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	}
	return NULL;
}

BOOL CProcess::FindProcess()
{
	m_hProcess = this->OpenProcess(NULL, windowTitle);
	if (m_hProcess)	m_bGameRunning = true;
	else m_bGameRunning = false;
	return m_bGameRunning;
}

BOOL CProcess::FindProcess(char *p_WindowTitle)
{
	windowTitle = p_WindowTitle;
	return this->FindProcess();
}


template<class TData>
BOOL CProcess::Write(DWORD address, TData data)
{
	if (!m_bGameRunning)
		return false;
	return WriteProcessMemory(m_hProcess,(void*)address,
		(void*)&data,sizeof(data),&bytesWritten);
}

template<class TData>
BOOL CProcess::Write(DWORD address, TData* data, DWORD size)
{
	if (!(m_bGameRunning && size))
		return false;
	return WriteProcessMemory(m_hProcess,(void*)address,
		(void*)data,size*sizeof(data[0]),&bytesWritten);
}

template<class TData>
TData CProcess::Read(TData* pAddress)
{
	DWORD BytesRead = 0;
	TData retVal;

	if (m_bGameRunning)
	{
		if (!ReadProcessMemory(m_hProcess, (void*)pAddress,
			(void *)&retVal, sizeof(retVal), &BytesRead)) return 0;
		return retVal;
	}
	return false;
}

template<class TData>
TData* CProcess::Read(TData* pAddress, DWORD size)
{
    DWORD bytes;
	TData* retVal = new TData[size];
	if (m_bGameRunning)
	{
		if (!ReadProcessMemory(m_hProcess, (void*)pAddress,
			(void *)retVal, size*sizeof(*retVal), &bytes)) return 0;
		return retVal;
	}
	return false;
}

template<class TData>
BOOL CProcess::CmpData(DWORD address, TData testData)
{
	TData actualData = Read((TData*) address);
	if (actualData==testData) return true;
	return false;
}

BYTE* CProcess::WriteXString(DWORD address, char* hexStr)				// In dire need of reworking
{
	DWORD strSize = (DWORD) strlen(hexStr);								// Size of string

	if (!m_bGameRunning || (strSize <= 0)) return false;				// 0 length or no game

	DWORD dataSize = 0;													// No of bytes to be written
	BYTE* array = new BYTE[1+(strSize/2)];								// 2 chars = 1 byte

	while (*hexStr) {													// For every character
		if (isxdigit(*hexStr)) {
			if (isxdigit(hexStr[1])) {
				sscanf(hexStr++, "%02x", (BYTE*) &array[dataSize++]);	// If it's hex scan for a byte
			}
		}
		hexStr++;
	}

	if (this->Write(address, array, dataSize)==TRUE)
		return array;
	return false;
}

DWORD CProcess::WriteString(DWORD address, char* ascStr)
{
	DWORD strSize = (DWORD)strlen(ascStr);
	if (strSize && this->Write(address, ascStr, 1+strSize))		// Add 1 to include the null?
		return address;
	return false;
}

DWORD CProcess::Alloc(int bytes)
{
	return (DWORD) VirtualAllocEx(m_hProcess, 0, bytes, MEM_COMMIT, PAGE_READWRITE);
}

DWORD CProcess::Free(DWORD address)
{
	return (DWORD) VirtualFreeEx(m_hProcess, 0, 0, MEM_RELEASE);
}

DWORD CProcess::SizeOfLast() {
	return bytesWritten;
}

char* CProcess::CmpString(DWORD address, char* ascStr)
{
	if (!m_bGameRunning) return false;
	DWORD charNo = (DWORD)strlen(ascStr);
	char* retStr = (char*) this->Read((char*)address, charNo);
	if (m_bGameRunning && retStr && !strncmp(retStr, ascStr, charNo)) return ascStr;
	return false;
}

DWORD CProcess::AddrDiff(DWORD origin, DWORD dest)
{
	return dest-(origin+0x04);
}

BOOL CProcess::WriteDiff(DWORD origin, DWORD dest)
{
	if (this->Write(origin, this->AddrDiff(origin, dest))) return true;
	return false;
}

BOOL CProcess::IsRunning()
{
	return m_bGameRunning;
}


BOOL CProcess::IsKeyPressed(int iKey)
{
	return ((GetAsyncKeyState(iKey) & 1) == 1);
}


BOOL CProcess::IsSpyRunning(char *p_WindowTitle)
{
	HANDLE hTmp = this->OpenProcess(NULL, "TRAINER SPY");
	if (hTmp)
	{
		CloseHandle(hTmp);
		return true;
	}
	return false;
}

__declspec(naked) void loadDll(void)
{
	_asm {
	// Save the flags and registers
	pushfd
	pushad
	// Placeholder for the string address and LoadLibrary
	push 0xDEADBEEF
	mov eax, 0xDEADBEEF
	// Call LoadLibrary with the string parameter
	call eax
	// Store the dll start address
	mov dword ptr ds:[0xDEADBEEF],eax
	// Restore the registers and flags
	popad
	popfd
	// Return control to the hijacked thread
	ret
	// Space for dll start address
	nop
	nop
	nop
	nop
   } 
} 
__declspec(naked) void loadDll_end(void) {} 

DWORD CProcess::InjectDll(char* dllName)
{
	void* stub					= NULL;
	void* dllString				= NULL;

	DWORD	stubLen		= (DWORD)loadDll_end - (DWORD)loadDll;
	FARPROC loadLibAddy	= GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

	// If nessacary allocate memory
	DWORD dllNameLength	= (DWORD)strlen(dllName)+1;
	dllString			= VirtualAllocEx(m_hProcess, NULL, stubLen+dllNameLength, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	stub				= (BYTE*)dllString + dllNameLength;

	// Fill in the place holders
	DWORD oldprot; 
	BYTE* loadLibRetVal = (BYTE*) stub + 23;
	VirtualProtect(loadDll, stubLen, PAGE_EXECUTE_READWRITE, &oldprot);
	memcpy((void *)((DWORD)loadDll + 3), &dllString, 4);
	memcpy((void *)((DWORD)loadDll + 8), &loadLibAddy, 4);
	memcpy((void *)((DWORD)loadDll + 16), &loadLibRetVal, 4);

	// Paste in the loading function
	WriteProcessMemory(m_hProcess, dllString, dllName, strlen(dllName), NULL);
	WriteProcessMemory(m_hProcess, stub, loadDll, stubLen, NULL);

	// Create a thread to load the dll
	HANDLE quickThread = CreateRemoteThread(m_hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)stub, (LPVOID)NULL, NULL, NULL);
	CloseHandle(quickThread);
	WaitForSingleObject(quickThread, INFINITE);

	// Get the dll location
	Sleep(3000);	// If it reads the memory too early, it crashes
	DWORD dllLocation = NULL, bytesRead = NULL;
	ReadProcessMemory(m_hProcess, (void*)loadLibRetVal, (BYTE*)&dllLocation, 4, &bytesRead);

	// Free the memory
	VirtualFreeEx(m_hProcess, dllString, stubLen+dllNameLength, MEM_DECOMMIT);

	// Return the location of the dll
	return dllLocation; 
} 

// Write single
template BOOL	CProcess::Write		<BYTE>	(DWORD	address, BYTE	data);
template BOOL	CProcess::Write		<WORD>	(DWORD	address, WORD	data);
template BOOL	CProcess::Write		<DWORD>	(DWORD	address, DWORD	data);
// Write array
template BOOL	CProcess::Write		<BYTE>	(DWORD	address, BYTE*	data,	DWORD size);
template BOOL	CProcess::Write		<WORD>	(DWORD	address, WORD*	data,	DWORD size);
template BOOL	CProcess::Write		<DWORD>	(DWORD	address, DWORD*	data,	DWORD size);
// Read Single
template BYTE	CProcess::Read		<BYTE>	(BYTE*	pAddress);
template WORD	CProcess::Read		<WORD>	(WORD*	pAddress);
template DWORD	CProcess::Read		<DWORD>	(DWORD*	pAddress);
// Read array
template BYTE*	CProcess::Read		<BYTE>	(BYTE*	pAddress, DWORD	size);
template WORD*	CProcess::Read		<WORD>	(WORD*	pAddress, DWORD	size);
template DWORD*	CProcess::Read		<DWORD>	(DWORD*	pAddress, DWORD	size);
// Compare Single
template BOOL	CProcess::CmpData	<BYTE>	(DWORD	address, BYTE data);