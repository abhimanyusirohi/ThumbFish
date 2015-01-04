#pragma once
class Utils
{
public:
	Utils(void);
	~Utils(void);

	static HRESULT GetSystemFolder(const KNOWNFOLDERID folderID, TCHAR* outPath);
	static void DrawErrorBitmap(HDC hDC, LPRECT lpRect);
	static void DrawMOLVersionIndicator(HDC hDC, bool v2000);
	static void DrawRecordCount(HDC hDC, RECT rect, int recordCount);
};
