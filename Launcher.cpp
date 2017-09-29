/*
MIT License

Copyright (c) 2017 Toomas Laasik

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "stdafx.h"
#include "Launcher.h"
#include <fstream>
#include <string>
#include <Windows.h>

/**
 * If this is defined then launcher is compiled for 64bit unity build, otherwise it's for 32bit build.
 * Release64 configuration has this defined, Release and Debug configuration does not.
 * Please note that win64 launcher is still 32 bit.
 */
//#define UNITY_WIN64_BUILD

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

// returns true if there is output_log and it contains "Crash!!!"
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

void DisplayError(int msgId)
{
	wchar_t titleBuf[256];
	wchar_t msgBuf[256];
	LoadString(GetModuleHandle(NULL), IDS_APP_TITLE, (LPWSTR)&titleBuf, 254);
	LoadString(GetModuleHandle(NULL), msgId, (LPWSTR)&msgBuf, 254);
	MessageBox(
		NULL,
		msgBuf,
		titleBuf,
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
		DisplayError(IDS_CANT_FIND_X64);
		return 1;
	}
#else
	if ((hFind = FindFirstFile(L"*.x86", &ffd)) == INVALID_HANDLE_VALUE)
	{
		DisplayError(IDS_CANT_FIND_X86);
		return 1;
	}
#endif

	// 2. copy files
	bool copyOk = true;
#ifdef UNITY_WIN64_BUILD
	// if it is Mono 2.0 or subset of it
	if (FileSize(L"Data\\Mono\\mono-x64.dll") > 0) {
		copyOk &= CopyIfNeeded(L"Data\\Mono\\mono-x64.dll", L"Data\\Mono\\mono.dll");
		copyOk &= CopyIfNeeded(L"Data\\Mono\\MonoPosixHelper-x64.dll", L"Data\\Mono\\MonoPosixHelper.dll");
		// XXX add more files here if needed for 64bit mono20 build
	}
	// otherwise expect Mono 4.6
	else {
		copyOk &= CopyIfNeeded(L"Data\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc-x64.dll", L"Data\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc.dll");
		copyOk &= CopyIfNeeded(L"Data\\MonoBleedingEdge\\EmbedRuntime\\MonoPosixHelper-x64.dll", L"Data\\MonoBleedingEdge\\EmbedRuntime\\MonoPosixHelper.dll");
		// XXX add more files here if needed for 64bit mono46 build
	}
	hFind = FindFirstFile(L"*.x64", &ffd);
#else
	if (FileSize(L"Data\\Mono\\mono-x86.dll") > 0) {
		copyOk &= CopyIfNeeded(L"Data\\Mono\\mono-x86.dll", (LPCWSTR)L"Data\\Mono\\mono.dll");
		copyOk &= CopyIfNeeded(L"Data\\Mono\\MonoPosixHelper-x86.dll", (LPCWSTR)L"Data\\Mono\\MonoPosixHelper.dll");
		// XXX add more files here if needed for 32bit mono20 build
	} else {
		copyOk &= CopyIfNeeded(L"Data\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc-x86.dll", L"Data\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc.dll");
		copyOk &= CopyIfNeeded(L"Data\\MonoBleedingEdge\\EmbedRuntime\\MonoPosixHelper-x86.dll", L"Data\\MonoBleedingEdge\\EmbedRuntime\\MonoPosixHelper.dll");
		// XXX add more files here if needed for 32bit mono46 build
	}
	hFind = FindFirstFile(L"*.x86", &ffd);
#endif
	FindClose(hFind);
	if (!copyOk) {
		DisplayError(IDS_COPY_FAIL);
		return 2;
	}

	// 3. start process
	PROCESS_INFORMATION pi;
	if(!StartProcess(&pi, ffd.cFileName))
	{
		DisplayError(
#ifdef UNITY_WIN64_BUILD
			IDS_CANT_RUN_X64
#else
			IDS_CANT_RUN_X86
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
		DisplayError(IDS_CRASH);
		return 4;
	}

	return 0;
}
