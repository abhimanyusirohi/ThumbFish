#pragma once

#define YOURS
#define CR		'\r'
#define LF		'\n'
#define	CRLF	"\r\n"

#define TEQUAL(a,b)	(_tcsicmp(a, b) == 0)
#define ALLOC_AND_COPY(src, dest, outlen) if(src != NULL) {	\
					size_t len = strlen(src) + 1;	\
					if(len > 1)	{					\
						dest = new char[len];		\
						dest[len - 1] = NULL;		\
						strcpy_s(dest, len, src);	\
					} if(outlen) *outlen = len; }

#define DeleteAndNull(x) { delete x; x = NULL; }

// supported formats
const enum ChemFormat { fmtUnknown, fmtMOLV2, fmtMOLV3, fmtRXNV2, fmtRXNV3, fmtSMILES, fmtSMARTS, fmtSDFV2, fmtSDFV3, 
	fmtRDFV2, fmtRDFV3, fmtCML,	fmtCDXML, fmtINCHI, fmtINCHIKEY, fmtEMF, fmtPNG, fmtPDF, fmtSVG, fmtMDLCT };

typedef std::basic_string<TCHAR> tstring;

const enum ProgressType { progressStart, progressWorking, progressDone };

struct CallbackEventArgs
{
	int total;
	int processed;
	PTSTR message;
	ProgressType type;

	CallbackEventArgs() : total(0), processed(0), message(NULL), type(progressStart) {}
};

typedef bool (*ProgressCallback)(LPVOID sender, CallbackEventArgs* e);

#include "Options.h"
#include "Buffer.h"
#include "CommonUtils.h"
#include "ExtractParam.h"

#pragma region API Function Pointers

// APIs
typedef bool (__cdecl *DrawFuncType)(HDC hDC, RECT rect, LPBUFFER buffer, LPOPTIONS options);
typedef int	 (__cdecl *GetPropertiesFuncType)(LPBUFFER buffer, TCHAR*** properties, LPOPTIONS options, bool searchNames);
typedef YOURS LPOUTBUFFER (__cdecl *ConvertToFuncType)(LPBUFFER buffer, ChemFormat outFormat, LPOPTIONS options);
typedef void (__cdecl *ExtractFuncType)(LPEXTRACTPARAMS params, LPOPTIONS options);

#pragma endregion
