#pragma once

#define YOURS
#define TEQUAL(a,b)	(_tcsicmp(a, _T(b)) == 0)
#define ALLOC_AND_COPY(src, dest, outlen) if(src != NULL) {	\
					size_t len = strlen(src) + 1;	\
					if(len > 1)	{					\
						dest = new char[len];		\
						dest[len - 1] = NULL;		\
						strcpy_s(dest, len, src);	\
					} if(outlen) *outlen = len; }

// supported formats
const enum ChemFormat { fmtUnknown, fmtMOLV2, fmtMOLV3, fmtRXNV2, fmtRXNV3, fmtSMILES, fmtSMARTS, 
	fmtSDF, fmtRDF, fmtCML,	fmtCDXML, fmtINCHI, fmtINCHIKEY, fmtEMF, fmtPNG, fmtPDF, fmtSVG, fmtMDLCT };

#include "Options.h"
#include "Buffer.h"
#include "CommonUtils.h"

#pragma region API Function Pointers

typedef bool (__cdecl *DrawFuncType)(HDC hDC, RECT rect, LPBUFFER buffer, LPOPTIONS options);
typedef int	 (__cdecl *GetPropertiesFuncType)(LPBUFFER buffer, TCHAR*** properties, LPOPTIONS options, bool searchNames);
typedef YOURS LPOUTBUFFER (__cdecl *ConvertToFuncType)(LPBUFFER buffer, ChemFormat outFormat, LPOPTIONS options);

#pragma endregion
