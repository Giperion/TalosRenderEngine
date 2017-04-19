#pragma once
class IRenderProgram
{
public:

	virtual void* GetBinaryCode() = 0;
	virtual UniString GetAllDrawCoPrograms() = 0;


	
};

