#pragma once

#define YOURS
#define TEQUAL(a,b) (_tcsicmp(a, _T(b)) == 0)
#define EXTLEN 10

// supported file extension. Converted to enum to speed up extension checks
const enum Extension { extUnknown, extMOL, extRXN, extSMI, extSMILES, extSMARTS, extSDF, extRDF, extCML };

struct OPTIONS
{
		bool			Changed;					// specifies whether options have changed
		bool			IsThumbnail;				// TRUE if the call is made by a thumbnail handler
		int				OutBufferSize;				// Size of the buffer returned by a method

		unsigned short  RenderMarginX;				// render-margins XY
		unsigned short	RenderMarginY;				// render-margins XY

		float			RenderRelativeThickness;	// render-relative-thickness D=1.0

		bool			RenderColoring;				// render-coloring BOOL D=False
		bool			RenderImplicitH;			// render-implicit-hydrogens-visible BOOL D=True
		bool			RenderShowAtomID;			// render-atom-ids-visible BOOL D=False
		bool			RenderShowBondID;			// render-bond-ids-visible BOOL D=False
		bool			RenderAtomBondIDFromOne;	// render-atom-bond-ids-from-one BOOL D=False

		COLORREF		RenderBaseColor;			// render-base-color STRING "r,g,b" D=BLACK
		COLORREF		RenderBackgroundColor;		// render-background-color STRING "r,g,b" D=TRANSPARENT

		unsigned short	RenderLabelMode;			// render-label-mode STRING "terminal-hetero"(D), "hetero", "none"
		unsigned short	RenderStereoStyle;			// render-stereo-style STRING "old"(D), "ext", "none"

		unsigned short	GridMaxMols;				// Maximum number of molecules to display in a Grid (both Preview and Thumbnail)
		unsigned short	GridMaxReactions;			// Maximum number of reactions to display in a Grid (both Preview and Thumbnail)

		char			RenderOutputExtension[EXTLEN];	// The extension to use when rendering to buffer or file (No DOT)
		unsigned short	RenderImageWidth;				// width of image when rendering to buffer or file
		unsigned short	RenderImageHeight;				// height of image when rendering to buffer or file
		unsigned short	MOLSavingMode;					// MDLMOL CT Version (0=auto, 1=2000, 2=3000)

public:
	OPTIONS() : Changed(true), RenderMarginX(20), RenderMarginY(20), RenderColoring(true), RenderImplicitH(true),
				RenderShowAtomID(false), RenderShowBondID(false), RenderAtomBondIDFromOne(true), 
				RenderBaseColor(RGB(0, 0, 0)), RenderBackgroundColor(RGB(255, 255, 255)), RenderLabelMode(0),
				RenderStereoStyle(1), RenderRelativeThickness(1.0), IsThumbnail(false), GridMaxMols(4), 
				GridMaxReactions(2), RenderImageWidth(300), RenderImageHeight(300), MOLSavingMode(0)
	{
		// default render output to buffer or file is in PNG format
		strcpy_s(RenderOutputExtension, EXTLEN, "png");
	}

	//TODO: ReadOptions and SaveOptions
};
typedef OPTIONS* LPOPTIONS;

struct Buffer
{
	char*			pData;
	long			DataLength;
	TCHAR			FileName[MAX_PATH];
	Extension		FileExtension;

public:
	Buffer() : pData(NULL), DataLength(0) {}
};

typedef Buffer BUFFER, *LPBUFFER;

typedef bool (__cdecl *DrawFuncType)(HDC hDC, LPRECT lpRect, LPBUFFER buffer, LPOPTIONS options);
typedef int	 (__cdecl *GetPropertiesFuncType)(LPBUFFER buffer, TCHAR*** properties, LPOPTIONS options, bool searchNames);
typedef YOURS char* (__cdecl *ConvertToFuncType)(LPBUFFER buffer, LPOPTIONS options);
