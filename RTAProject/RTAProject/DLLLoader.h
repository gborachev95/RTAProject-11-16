#include <cstdio>
#include <string>

#if defined(_MSC_VER)
// WINDOWS


// Normally you should include platform specific core header files
// in a central place in a precompiled header file but this time
// I include it locally for simplicity.
#include <windows.h>
#define DLL_HANDLE HMODULE
#define MyLoadLibrary LoadLibrary
#define MyFreeLibrary FreeLibrary
#define MyGetProcAddress GetProcAddress

inline std::string GetModulePathByAddr(void* addr)
{
	HMODULE module;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)addr, &module);
	// Using ugly ansi version without error handling to
	// make it easier to write cross-platform example.
	char path[MAX_PATH];
	//LPWSTR lPath;
	GetModuleFileName(module, (LPWSTR)path, sizeof(path));

	return path;
}
#endif

template <typename DLLInterfaceClass>
class TDLLLoader
{
public:
	enum ESearchPath
	{
		ESearchPath_Normal,
		ESearchPath_RelativeToCurrentModule
	};

	TDLLLoader(const char* dll_path, const char* interface_getter_funcname,
		ESearchPath search_path = ESearchPath_RelativeToCurrentModule)
		: m_DLLHandle(NULL)
		, m_Interface(NULL)
		, m_InterfaceGetterFuncname(interface_getter_funcname)
	{
		if (search_path == ESearchPath_Normal)
		{
			this->m_DLLPath = dll_path;
		}
		else
		{
			static char dummy = 0;
			this->m_DLLPath = GetModulePathByAddr(&dummy);
			size_t filename_pos = this->m_DLLPath.find_last_of("/\\");
			if (filename_pos == std::string::npos)
				filename_pos = 0;
			else
				filename_pos += 1;
			this->m_DLLPath.resize(filename_pos);
			this->m_DLLPath += dll_path;
		}
	}

	~TDLLLoader()
	{
		UnloadDLL();
	}

	// Actually this is the only method you should call... This does the DLL loading
	// too for you. If you never call this method (and you don't call LoadDLL
	// explicitly) then the DLL is never loaded, this is what ppl call delay loading.
	// This is the programmatic delay loading, Visual C++ has support for automatic
	// delay loading in case of implicit DLL linking but I like this way more because
	// it's easy to make crossplatform and I find it easier to debug.
	DLLInterfaceClass* GetInterface()
	{
		if (!this->m_Interface)
			LoadDLL();
		return this->m_Interface;
	}

	bool LoadDLL()
	{
		if (this->m_DLLHandle)
			return true;

		printf("Loading DLL: %s...\n", this->m_DLLPath.c_str());

		// This is not correct because we call the ansi version of this function,
		// the filename is not unicode aware! I always use utf8 internally in
		// my programs and convert them to utf-16 before I call the 'W' version
		// of the windows functions. Utf-8 is nicer to use in C++ source (unlike
		// the utf-16 strings like L"string") and utf-8 is native for unix like
		// systems so porting becomes easy. Now you have to deal with this
		// because I opt for simplicity in this example program! :P
		this->m_DLLHandle = MyLoadLibrary((LPCWSTR)this->m_DLLPath.c_str());
		if (!this->m_DLLHandle)
		{
			fprintf(stderr, "Error loading DLL: %s\n", this->m_DLLPath.c_str());
			return false;
		}

		// exported dll function pointer type
		typedef DLLInterfaceClass* (*PF_GetDLLInterface)();
		PF_GetDLLInterface pfunc = (PF_GetDLLInterface)MyGetProcAddress(this->m_DLLHandle, this->m_InterfaceGetterFuncname.c_str());
		if (!pfunc)
		{
			fprintf(stderr, "Error getting address of function '%s' in DLL: %s\n", this->m_InterfaceGetterFuncname.c_str(), this->m_DLLPath.c_str());
			UnloadDLL();
			return false;
		}

		this->m_Interface = pfunc();
		if (!this->m_Interface)
		{
			fprintf(stderr, "Interface getter function '%s' returned NULL in DLL '%s'\n", this->m_InterfaceGetterFuncname.c_str(), this->m_DLLPath.c_str());
			UnloadDLL();
			return false;
		}

		return true;
	}

	void UnloadDLL()
	{
		if (!this->m_DLLHandle)
			return;
		printf("Unloading DLL: %s...\n", this->m_DLLPath.c_str());
		MyFreeLibrary(this->m_DLLHandle);
		this->m_DLLHandle = NULL;
		this->m_Interface = NULL;
	}

	bool IsLoaded() const
	{
		return NULL != this->m_DLLHandle;
	}

private:
	DLL_HANDLE m_DLLHandle;
	DLLInterfaceClass* m_Interface;
	std::string m_DLLPath;
	std::string m_InterfaceGetterFuncname;
};