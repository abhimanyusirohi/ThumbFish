#pragma once
class Utils
{
public:
	Utils(void);
	~Utils(void);

	static HRESULT GetSystemFolder(const KNOWNFOLDERID folderID, TCHAR* outPath);
};

