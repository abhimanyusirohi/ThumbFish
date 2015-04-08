#pragma once

#include <map>

// To compare the values inside WCHAR pointers in map
struct CompareWCHAR
{
	bool operator()(const WCHAR* a, const WCHAR* b)
	{
		return std::wcscmp(a, b) < 0;
	}
};

typedef std::map<WCHAR*, WCHAR*, CompareWCHAR> MAP_WSWS;

class BrowseEventArgs
{
public:
	LPOUTBUFFER MolData;	// molecule data in source file specific format, e.g. MOL in SDF
	MAP_WSWS Properties;	// per record properties in file such as SDF
	ChemFormat DataFormat;	// Format of data in MolData

	BrowseEventArgs() : MolData(NULL), DataFormat(fmtUnknown) {}
	BrowseEventArgs(BrowseEventArgs& b)
	{
		this->MolData = b.MolData;
		this->Properties = b.Properties;
		this->DataFormat = b.DataFormat;
	}
};

typedef bool (*BrowseCallback)(LPVOID instance, BrowseEventArgs* e);

class BrowseParams
{
public:
	char SourceFile[MAX_PATH];
	ChemFormat SourceFormat;

	LPVOID caller;
	BrowseCallback callback;
};

typedef BrowseParams BROWSEPARAMS, *LPBROWSEPARAMS;
