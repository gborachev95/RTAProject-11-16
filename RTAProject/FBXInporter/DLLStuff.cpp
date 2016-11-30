#include "stdafx.h"
#include "DLLStuff.h"


// Implementation of the DLL interface inside the DLL.
class CDLLInterface : public I_DLLInterface
{
public:
	virtual void test(int * a) override;
};


void CDLLInterface::test(int * a)
{
	*a += 1;
}


#if defined(_MSC_VER)
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __attribute__ ((visibility ("default")))
#endif

//We use the extern "C" to prevent the compiler from mangling
// the name of this function in the export table of the DLL because we want
// to load it "manually" in another module and its easier to use GetProcAddress()
// with an unmangled name like "GetDLLInterface".
extern "C" DLL_API I_DLLInterface* GetDLLInterface()
{
	// Of course you can instantiate CDLLInterface wherever you want,
	// for example in your DLLMain() if you like that more.
	static CDLLInterface g_DLLInterface;
	return &g_DLLInterface;
}