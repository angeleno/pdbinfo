// pdbinfo.cpp : Defines the entry point for the console application.
//
#include <atlbase.h>
#include <dia2.h>

#include <exception>
#include <iostream>
#include <string>

//------------------------------------------------------------------------------------
static void Fatal(const char* msg)
{
    std::cout << msg << std::endl;
    throw std::exception(msg);
}

//------------------------------------------------------------------------------------
class ComInitializer
{
public:
    ComInitializer()
    {
        HRESULT hr = CoInitialize(NULL);
        m_initialized = SUCCEEDED(hr);
    }

    ~ComInitializer()
    {
        CoUninitialize();
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

private:
    bool m_initialized{ false };
};


//------------------------------------------------------------------------------------
//  https://stackoverflow.com/a/22848342/916549
//
static std::string ToString(GUID* guid)
{
    char guid_string[37]; // 32 hex chars + 4 hyphens + null terminator

    snprintf(guid_string, sizeof(guid_string) / sizeof(guid_string[0]),
        "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        guid->Data1, guid->Data2, guid->Data3,
        guid->Data4[0], guid->Data4[1], guid->Data4[2],
        guid->Data4[3], guid->Data4[4], guid->Data4[5],
        guid->Data4[6], guid->Data4[7]);

    return guid_string;
}

//------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc != 2)
        Fatal("invalid #arguments passed, 1 arg expected");

    ComInitializer initializer;
    if (initializer.IsInitialized())
    {
        CComPtr<IDiaDataSource> pSource;
        HRESULT hr = CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER,
                                      __uuidof(IDiaDataSource), (void**)&pSource);

        wchar_t wszFilename[_MAX_PATH];
        mbstowcs(wszFilename, argv[1], _MAX_PATH);
        if (FAILED(pSource->loadDataFromPdb(wszFilename)))
            Fatal("loadDataFromPdb failed");

        CComPtr<IDiaSession> pSession;
        if (FAILED(pSource->openSession(&pSession)))
            Fatal("failed to open Session");

        CComPtr<IDiaSymbol> pSymbol;
        if (FAILED(pSession->get_globalScope(&pSymbol)))
            Fatal("failed to get globalScope");

        GUID pdbGuid;
        if (FAILED(pSymbol->get_guid(&pdbGuid)))
            Fatal("failed to get guid");

        OLECHAR szGUID[64] = { 0 };
        StringFromGUID2(pdbGuid, szGUID, 64);
        std::cout << "PDB file: " << argv[1] << std::endl;
        std::cout << "GUID is: " << ToString(&pdbGuid) << std::endl;
    }

    return 0;
}

