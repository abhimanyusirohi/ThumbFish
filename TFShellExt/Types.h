#pragma once

#define YOURS
#define CR		'\r'
#define LF		'\n'
#define	CRLF	"\r\n"

#define TEQUAL(a,b)	(_tcsicmp(a, b) == 0)
#define ALLOC_AND_COPY(src, dest, outlen) if(src != NULL) {		\
					size_t len = strlen(src) + 1;				\
					if(len > 1)	{								\
						PCHAR pDest = new char[len];			\
						pDest[len - 1] = NULL;					\
						strcpy_s(pDest, len, src);				\
						dest = pDest;							\
						if(outlen) *outlen = len;				\
					}}

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

// Parameters required to execute a command
typedef struct CommandParams
{
	int			CommandID;				// command id of operation
	LPBUFFER	Buffer;					// buffer containing data to work on
	PVOID		Param;					// additional parameter required to execute this command

	CommandParams(int id, LPBUFFER buffer = NULL, LPVOID param = NULL) :
		CommandID(id), Buffer(buffer), Param(param) {}
} COMMANDPARAMS, *LPCOMMANDPARAMS;

typedef struct DrawParams
{
	HDC hDC;
	RECT targetRect;

	DrawParams(HDC hdc, RECT rect) : hDC(hdc), targetRect(rect) {}
} DRAWPARAMS, *LPDRAWPARAMS;

// API
const enum Command { cmdVersion, cmdDraw, cmdGetProperties, cmdConvert, cmdExtract, cmdAromatize, cmdDearomatize, 
					 cmdCleanup, cmdValidate, cmdNormalize, cmdFoldHydrogens, cmdUnfoldHydrogens };
typedef YOURS LPOUTBUFFER (__cdecl *ExecuteFuncType)(LPCOMMANDPARAMS command, LPOPTIONS options);
