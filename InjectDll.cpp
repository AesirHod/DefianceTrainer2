#include "InjectDll.h"

DWORD GetTargetProcessIdFromProcname(char *procName);
DWORD GetTargetThreadIdFromProcname(char *procName);

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

DWORD Inject(char* PROC_NAME, char* DLL_NAME, void* loaderAddress, DWORD* bytesWritten)
{
	DWORD CREATE_THREAD_ACCESS	=	PROCESS_CREATE_THREAD	|	PROCESS_QUERY_INFORMATION	|
									PROCESS_VM_OPERATION	|	PROCESS_VM_WRITE	|
									PROCESS_VM_READ			|	PROCESS_VM_OPERATION;
	DWORD ORDINARY_ACCESS		=	PROCESS_VM_WRITE		|	PROCESS_VM_OPERATION;

	bool allocStub				= loaderAddress==NULL;
	void* stub					= NULL;
	void* dllString				= loaderAddress;

	DWORD stubLen		= (DWORD)loadDll_end - (DWORD)loadDll;
	FARPROC loadLibAddy	= GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	DWORD procID		= GetTargetProcessIdFromProcname(PROC_NAME);
	HANDLE hProcess		= OpenProcess(CREATE_THREAD_ACCESS, false, procID); 

	// If nessacary allocate memory
	DWORD dllNameLength	= (DWORD)strlen(DLL_NAME)+1;
	if (bytesWritten) *bytesWritten = stubLen+dllNameLength;
	if (allocStub) {
		dllString		= VirtualAllocEx(hProcess, NULL, stubLen+dllNameLength, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	}
	stub				= (BYTE*)dllString + dllNameLength;

	// Fill in the place holders
	DWORD oldprot; 
	DWORD loadLibRetVal = (DWORD) stub + 23;
	VirtualProtect(loadDll, stubLen, PAGE_EXECUTE_READWRITE, &oldprot);
	memcpy((void *)((DWORD)loadDll + 3), &dllString, 4);
	memcpy((void *)((DWORD)loadDll + 8), &loadLibAddy, 4);
	memcpy((void *)((DWORD)loadDll + 16), &loadLibRetVal, 4);

	// Paste in the loading function
	WriteProcessMemory(hProcess, dllString, DLL_NAME, strlen(DLL_NAME), NULL);
	WriteProcessMemory(hProcess, stub, loadDll, stubLen, NULL);

	// Create a thread to load the dll
	HANDLE quickThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)/*loadLibAddy*/stub, (LPVOID)/*dllString*/NULL, NULL, NULL);
	CloseHandle(quickThread);
	WaitForSingleObject(quickThread, INFINITE);

	// Get the dll location
	Sleep(3000);	// If it reads the memory too early, it crashes
	DWORD dllLocation = NULL, bytesRead = NULL;
	ReadProcessMemory(hProcess, (void*)loadLibRetVal, (BYTE*)&dllLocation, 4, &bytesRead);

	// Free the memory
	if (allocStub)
		VirtualFreeEx(hProcess, dllString, stubLen+dllNameLength, MEM_DECOMMIT);

	// Close the handle
	CloseHandle(hProcess);

	// Return the location of the dll
	return dllLocation; 
} 

DWORD GetTargetProcessIdFromProcname(char *procName) 
{ 
   PROCESSENTRY32 pe; 
   HANDLE thSnapshot; 
   BOOL retval, ProcFound = false; 

   thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

   if(thSnapshot == INVALID_HANDLE_VALUE) 
   { 
      MessageBox(NULL, "Error: unable to create toolhelp snapshot", "Loader", NULL); 
      return false; 
   } 

   pe.dwSize = sizeof(PROCESSENTRY32); 

    retval = Process32First(thSnapshot, &pe); 

   while(retval) 
   { 
      if(StrStrI(pe.szExeFile, procName) ) 
      { 
         ProcFound = true; 
         break; 
      } 

      retval    = Process32Next(thSnapshot,&pe); 
      pe.dwSize = sizeof(PROCESSENTRY32); 
   } 

   CloseHandle(thSnapshot); 
   return pe.th32ProcessID; 
} 

DWORD GetTargetThreadIdFromProcname(char *procName) 
{ 
   PROCESSENTRY32 pe; 
   HANDLE thSnapshot, hProcess; 
   BOOL retval, ProcFound = false; 
   void* pTID;
   DWORD threadID; 

   thSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

   if(thSnapshot == INVALID_HANDLE_VALUE) 
   { 
      MessageBox(NULL, "Error: unable to create toolhelp snapshot", "Loader", NULL); 
      return false; 
   } 

   pe.dwSize = sizeof(PROCESSENTRY32); 

    retval = Process32First(thSnapshot, &pe); 

   while(retval) 
   { 
      if(StrStrI(pe.szExeFile, procName) ) 
      { 
         ProcFound = true; 
         break; 
      } 

      retval    = Process32Next(thSnapshot,&pe); 
      pe.dwSize = sizeof(PROCESSENTRY32); 
   } 

   CloseHandle(thSnapshot); 
    
   _asm { 
      mov eax, fs:[0x18] 
      add eax, 36 
      mov [pTID], eax 
   } 

   hProcess = OpenProcess(PROCESS_VM_READ, false, pe.th32ProcessID); 
   ReadProcessMemory(hProcess, (const void*)pTID, &threadID, 4, NULL); 
   CloseHandle(hProcess); 

   return threadID; 
}
