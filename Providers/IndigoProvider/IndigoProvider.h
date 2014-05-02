// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the INDIGOPROVIDER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// INDIGOPROVIDER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef INDIGOPROVIDER_EXPORTS
#define INDIGOPROVIDER_API extern "C" __declspec(dllexport)
#else
#define INDIGOPROVIDER_API extern "C" __declspec(dllimport)
#endif

const enum ReturnObjectType { SingleMol, MultiMol };

INDIGOPROVIDER_API bool Draw(HDC hDC, RECT rect, LPBUFFER buffer, LPOPTIONS options);
INDIGOPROVIDER_API int GetProperties(LPBUFFER buffer, TCHAR*** properties, LPOPTIONS options, bool searchNames);
INDIGOPROVIDER_API char* ConvertTo(LPBUFFER buffer, LPOPTIONS options);

extern "C" __declspec(dllexport) UINT __stdcall RefreshIcons(MSIHANDLE hInstall);

void AddProperty(TCHAR*** properties, int startIndex, TCHAR* name, TCHAR* value);
std::string GetData(LPBUFFER buffer);
int ReadBuffer(LPBUFFER buffer, ReturnObjectType* type);
void SetIndigoOptions(LPOPTIONS options);
INT64 GetPropFlagsForFile(const TCHAR* fileName, int* numProps);
void DrawErrorBitmap(HDC hDC, LPRECT lpRect);
