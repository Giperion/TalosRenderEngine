#pragma once
class IRenderProgram
{
public:
	UnicodeString Name;

	IRenderProgram();
	~IRenderProgram();

	void* GetBinaryCode();
	
};

