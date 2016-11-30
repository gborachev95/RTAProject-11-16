#pragma once

// DLL interface, this header is used by both the DLL and the module
// that wants to access the functionalities of the DLL.
class I_DLLInterface
{
public:
	virtual void test(int  *a) = 0;

	// TODO: Add as many functions as you wish...
};