#pragma once
#include "unicode\unistr.h"
class Log
{
public:
	void SetHandleFile();
	void PrintMsg(UnicodeString& format, ...);
	static Log* GetInstance();
	~Log();
private:
	static Log* instance;
	LPWSTR MsgBuffer;
	HANDLE stdOut;
	CRITICAL_SECTION log_locker;
	Log();

};

