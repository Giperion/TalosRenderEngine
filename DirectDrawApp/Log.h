#pragma once
#include "unicode\unistr.h"
class Log
{
public:
	void SetHandleFile();
	void PrintMsg(LPCWSTR format, ...);
	void PrintMsg(UnicodeString& format, ...);
	static Log* GetInstance();
	~Log();
private:
	static Log* instance;
	LPWSTR MsgBuffer;
	HANDLE stdOut;
	CRITICAL_SECTION log_locker;
	Log();

	void PrintMsg(UnicodeString& format, va_list Args);
};

