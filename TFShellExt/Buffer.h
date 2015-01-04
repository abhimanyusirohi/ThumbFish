/**
	This file contains Buffer and OutBuffer classes for API operations.
*/

//-----------------------------------------------------------------------------
// <Buffer>
// Input Buffer used to send data to the API.
//-----------------------------------------------------------------------------
typedef struct Buffer
{
	PVOID		pData;				// this may contain a single or multiple records
	size_t		DataLength;			// length of data in pData
	ChemFormat	DataFormat;			// format of data in pData
	int			recordCount;		// approximate number of records in pData

	Buffer(bool releaseData = true) : pData(NULL), DataLength(0), DataFormat(fmtUnknown), 
		recordCount(1), m_releaseData(releaseData) {}
	~Buffer() { if(m_releaseData) DeleteAndNull(pData); }

private:
	bool		m_releaseData;		// Default: True. Set to False in ctor to avoid releasing pData
} BUFFER, *LPBUFFER;

//-----------------------------------------------------------------------------
// <OutBuffer>
// Output Buffer used to get values back from the API.
//-----------------------------------------------------------------------------
typedef struct OutBuffer
{
	PVOID	pData;
	size_t	DataLength;

	OutBuffer() : pData(NULL), DataLength(0) {}
} OUTBUFFER, *LPOUTBUFFER;
