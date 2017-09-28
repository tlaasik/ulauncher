
#include "stdafx.h"
#include "Launcher.h"
#include <fstream>
#include <string>
#include <Windows.h>

//#define UNITY_WIN64_BUILD
//#define UNITY_5

// returns file size or -1 on error (call GetLastError for details). It can be up to 31 bits
int FileSize(LPCTSTR name)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
		return -1;
	return fad.nFileSizeLow;
}

// returns false only if the file needed to be copied, but it failed
bool CopyIfNeeded(LPCTSTR existingFileName, LPCTSTR newFileName)
{
	int existingFileSize = FileSize(existingFileName);
	int newFileSize = FileSize(newFileName);
	
	if (newFileSize > 0 && (newFileSize == existingFileSize || existingFileSize < 0))
		return true;

	return CopyFile(existingFileName, newFileName, 0) ? true : false;
}

// returns true if process was started, otherwise false (call GetLastError for details)
bool StartProcess(PROCESS_INFORMATION *pi, LPWSTR processName)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(pi, sizeof(pi));

	// Start the child process. 
	return CreateProcess(NULL,   // No module name (use command line)
		processName,    // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		pi              // Pointer to PROCESS_INFORMATION structure
	) ? true : false;
}

bool CheckIfCrashed()
{
	std::ifstream infile("Data\\output_log.txt");
	
	bool found = false;
	for (std::string line; getline(infile, line); )
	{
		if (line.find("Crash!!!") != std::string::npos)
		{
			found = true;
			break;
		}
	}
	infile.close();
	return found;
}

void DisplayError(LPCWSTR msg)
{
	MessageBox(
		NULL,
		msg,
		L"Launcher",
		MB_TOPMOST | MB_ICONERROR | MB_OK
	);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	// 1. find executable
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
#ifdef UNITY_WIN64_BUILD
	if ((hFind = FindFirstFile(L"*.x64", &ffd)) == INVALID_HANDLE_VALUE)
	{
		DisplayError(L"Can't find .x64 executable");
		return 1;
	}
#else
	if ((hFind = FindFirstFile(L"*.x86", &ffd)) == INVALID_HANDLE_VALUE)
	{
		DisplayError(L"Can't find .x86 executable");
		return 1;
	}
#endif

	// 2. copy files
	bool copyOk = true;
#ifdef UNITY_WIN64_BUILD
#ifdef UNITY_5
	copyOk &= CopyIfNeeded(L"Data\\Mono\\mono-x64.dll", L"Data\\Mono\\mono.dll");
	copyOk &= CopyIfNeeded(L"Data\\Mono\\MonoPosixHelper-x64.dll", L"Data\\Mono\\MonoPosixHelper.dll");
#else
	copyOk &= CopyIfNeeded(L"Data\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc-x64.dll", L"Data\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc.dll");
	copyOk &= CopyIfNeeded(L"Data\\MonoBleedingEdge\\EmbedRuntime\\MonoPosixHelper-x64.dll", L"Data\\MonoBleedingEdge\\EmbedRuntime\\MonoPosixHelper.dll");
#endif
	hFind = FindFirstFile(L"*.x64", &ffd);
#else
#ifdef UNITY_5
	copyOk &= CopyIfNeeded(L"Data\\Mono\\mono-x86.dll", (LPCWSTR)L"Data\\Mono\\mono.dll");
	copyOk &= CopyIfNeeded(L"Data\\Mono\\MonoPosixHelper-x86.dll", (LPCWSTR)L"Data\\Mono\\MonoPosixHelper.dll");
#else
	copyOk &= CopyIfNeeded(L"Data\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc-x86.dll", L"Data\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc.dll");
	copyOk &= CopyIfNeeded(L"Data\\MonoBleedingEdge\\EmbedRuntime\\MonoPosixHelper-x86.dll", L"Data\\MonoBleedingEdge\\EmbedRuntime\\MonoPosixHelper.dll");
#endif
	hFind = FindFirstFile(L"*.x86", &ffd);
#endif
	FindClose(hFind);
	if (!copyOk) {
		DisplayError(L"File copy fail");
		return 2;
	}

	// 3. start process
	PROCESS_INFORMATION pi;
	if(!StartProcess(&pi, ffd.cFileName))
	{
		DisplayError(
#ifdef UNITY_WIN64_BUILD
			L"Can't run \".x64\" executable"
#else
			L"Can't run \".x86\" executable"
#endif
		);
		return 3;
	}

	// Wait until child process exits
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if(CheckIfCrashed()) {
		DisplayError(L"Game crashed. Please find and send Data/output_log.txt to developers");
		return 4;
	}

	return 0;
}
