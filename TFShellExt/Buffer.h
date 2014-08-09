/**
	This file contains Buffer and OutBuffer classes for API operations.
*/

/** Input Buffer used to send values to the API */
class Buffer
{
public:
	PCHAR		pData;
	size_t		DataLength;
	TCHAR		FileName[MAX_PATH];
	ChemFormat	DataFormat;
	PVOID		Extra;

	//unsigned short	DataVersion;	// usually MDL/RXN version. (0=unknown, 1=V2000, 2=V3000)
	//int				TotalRecords;	// approx number of records

public:
	Buffer() : pData(NULL), DataLength(0), DataFormat(fmtUnknown), Extra(NULL) {}
};

typedef Buffer BUFFER, *LPBUFFER;

/** Output Buffer used to get values back from the API */
class OutBuffer
{
public:
	PCHAR	pData;
	size_t	DataLength;

public:
	OutBuffer() : pData(NULL), DataLength(0) {}
	~OutBuffer() { delete[] pData; }
};

typedef OutBuffer OUTBUFFER, *LPOUTBUFFER;
