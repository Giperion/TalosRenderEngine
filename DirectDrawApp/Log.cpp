#include "stdafx.h"
#include "Log.h"


Log* Log::instance = nullptr;

Log::Log()
{
	stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	MsgBuffer = new wchar_t[512];
	ZeroMemory(MsgBuffer, sizeof(wchar_t)* 512);
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
}


void Log::PrintMsg(UnicodeString& format, ...)
{
	va_list args;
	size_t unusedBytes;
	UnicodeString* testptr = &format;
	
	DWORD writtenChar;
	LPCVOID buf = format.getTerminatedBuffer();
	DWORD len = format.length();
	va_start(args, format);

	if (FAILED(
		StringCbVPrintfExW(MsgBuffer, 512 * sizeof(wchar_t), NULL, &unusedBytes, 0, (LPWSTR)buf, args)))
	{
		wsprintf(MsgBuffer, L"Internal API error: PrintMsg\n");
		unusedBytes = 966;
	}
	va_end(args);

	//calc length
	int newLen = (1024 - unusedBytes) / 2;

	BOOL result = WriteConsoleW(stdOut, MsgBuffer, newLen, &writtenChar, NULL);
	if (!result)
	{
		WriteFile(stdOut, buf, len * sizeof(wchar_t), &writtenChar, NULL);
	}
}
