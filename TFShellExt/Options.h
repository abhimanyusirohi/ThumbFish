/**
	Contains and handles user options for API operations.
*/

#define EXTLEN 10

class Options
{
public:
	bool			Changed;					// specifies whether options have changed
	bool			IsThumbnail;				// TRUE if the call is made by a thumbnail handler

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

	unsigned short	RenderImageWidth;				// width of image when rendering to buffer or file
	unsigned short	RenderImageHeight;				// height of image when rendering to buffer or file
	unsigned short	MOLSavingMode;					// MDLMOL CT Version (0=auto, 1=2000, 2=3000)

	char* OutWarning1;							// Chemical Warning 1
	char* OutWarning2;							// Chemical Warning 2
	char* OutWarning3;							// Chemical Warning 3

public:
	Options() : Changed(true), RenderMarginX(20), RenderMarginY(20), RenderColoring(true), RenderImplicitH(true),
				RenderShowAtomID(false), RenderShowBondID(false), RenderAtomBondIDFromOne(true), 
				RenderBaseColor(RGB(0, 0, 0)), RenderBackgroundColor(RGB(255, 255, 255)), RenderLabelMode(0),
				RenderStereoStyle(1), RenderRelativeThickness(1.0), IsThumbnail(false), GridMaxMols(4), 
				GridMaxReactions(2), RenderImageWidth(300), RenderImageHeight(300), MOLSavingMode(0), 
				OutWarning1(NULL), OutWarning2(NULL), OutWarning3(NULL)
	{
	}

	~Options()
	{
		delete[] OutWarning1;
		delete[] OutWarning2;
		delete[] OutWarning3;
	}
	//TODO: ReadOptions and SaveOptions
};

typedef Options OPTIONS, *LPOPTIONS;
