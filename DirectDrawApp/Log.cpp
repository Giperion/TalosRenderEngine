#include "stdafx.h"
#include "Log.h"


Log* Log::instance = nullptr;

Log::Log()
{
	stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	MsgBuffer = new wchar_t[512];
	ZeroMemory(MsgBuffer, sizeof(wchar_t)* 512);
	InitializeCriticalSectionAndSpinCount(&log_locker, 100);
}

Log* Log::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new Log();
	}
	return instance;
}

void Log::SetHandleFile()
{
	HANDLE fileStd = CreateFile(L"C:\\Projects\\Test\\321.txt", GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	UnicodeString newStr = L"A new String....";
	if (fileStd == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		return;
	}
	DWORD writtenChar;
	WriteFile(fileStd, newStr.getTerminatedBuffer(), newStr.length() * sizeof(wchar_t), &writtenChar, NULL);

	BOOL res = SetStdHandle(STD_OUTPUT_HANDLE, fileStd);
	//time to fuck them all!
}


Log::~Log()
{
	delete[] MsgBuffer;
	DeleteCriticalSection(&log_locker);
}


void Log::PrintMsg(UnicodeString& format, ...)
{
	va_list ArgsList;
	va_start(ArgsList, format);
	PrintMsg(format, ArgsList);
	va_end(ArgsList);
}

void Log::PrintMsg(LPCWSTR format, ...)
{
	UnicodeString UniFormat (format);
	va_list ArgsList;
	va_start(ArgsList, format);
	PrintMsg(UniFormat, ArgsList);
	va_end(ArgsList);
}

void Log::PrintMsg(UnicodeString& format, va_list Args)
{
	size_t unusedBytes;

	DWORD writtenChar;
	LPCVOID buf = format.getTerminatedBuffer();
	DWORD len = format.length();
	EnterCriticalSection(&log_locker);
	if (FAILED(
		StringCbVPrintfExW(MsgBuffer, 512 * sizeof(wchar_t), NULL, &unusedBytes, 0, (LPWSTR)buf, Args)))
	{
		wsprintf(MsgBuffer, L"Internal API error: PrintMsg\n");
		unusedBytes = 966;
	}

	//calc length
	int newLen = (1024 - unusedBytes) / 2;

	//write end line
	if (newLen < 510)
	{
		MsgBuffer[newLen] = '\r';
		MsgBuffer[newLen + 1] = '\n';
		newLen += 2;
	}

	BOOL result = WriteConsoleW(stdOut, MsgBuffer, newLen, &writtenChar, NULL);
	LeaveCriticalSection(&log_locker);
	if (!result)
	{
		WriteFile(stdOut, buf, len * sizeof(wchar_t), &writtenChar, NULL);
	}
}

