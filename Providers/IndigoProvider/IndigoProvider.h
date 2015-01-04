// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the INDIGOPROVIDER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// INDIGOPROVIDER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef INDIGOPROVIDER_EXPORTS
#define INDIGOPROVIDER_API extern "C" __declspec(dllexport)
#else
#define INDIGOPROVIDER_API extern "C" __declspec(dllimport)
#endif

const enum ReturnObjectType { SingleMol, MultiMol };

const enum PropFlags {
	propName = 0x1, propNumAtoms = 0x2, propNumBonds = 0x4, propImplH = 0x8, propHeavyAtoms = 0x10, propGrossFormula = 0x20,
	propMolWeight = 0x40, propMostAbundantMass = 0x80, propMonoIsotopicMass = 0x100, propIsChiral = 0x200, propHasCoord = 0x400,
	propHasZCoord = 0x800, propSmiles = 0x1000, propCanonicalSmiles = 0x2000, propLayeredCode = 0x4000, propInChI = 0x8000, 
	propInChIKey = 0x10000, propDataVersion = 0x20000, propSSSR = 0x40000
};

/**
	Specifies the available properties for each format type.
	Update this array when a new format is added to the ChemFormat enum.
*/
const INT64 FormatPropInfo[18][2] = { 
	{ 0, 0 },	// fmtUnknown
	{ 18, ULONG_MAX }, // fmtMOLV2 - all properties
	{ 18, ULONG_MAX }, // fmtMOLV3 - all properties
	{ 4, propName | propSmiles | propIsChiral | propDataVersion }, // fmtRXNV2
	{ 4, propName | propSmiles | propIsChiral | propDataVersion }, // fmtRXNV3
	{ 8, propName | propNumAtoms | propNumBonds | propHeavyAtoms | propSmiles | propCanonicalSmiles | propLayeredCode | propSSSR }, // extSMILES
	{ 7, propName | propNumAtoms | propNumBonds | propHeavyAtoms | propGrossFormula | propSmiles | propSSSR }, // extSMARTS
	{ 0, 0 },	// extSDF - no properties
	{ 0, 0 },	// extRDF - no properties
	{ 0, 0 },	// extCML - no properties
	{ 0, 0 },	// extCDXML - no properties
	{ 0, 0 },	// extINCHI - no properties
	{ 0, 0 },	// extINCHIKEY - no properties
	{ 0, 0 },	// fmtEMF - no properties
	{ 0, 0 },	// fmtPNG - no properties
	{ 0, 0 },	// extPDF - no properties
	{ 0, 0 },	// extSVG - no properties
	{ 0, 0 }	// extMDLCT - no properties
};

const char VersionString[] = "Indigo Provider v1.11";

INDIGOPROVIDER_API LPOUTBUFFER Execute(LPCOMMANDPARAMS command, LPOPTIONS options);

// main command methods called by Execute
bool		Draw(LPCOMMANDPARAMS params, LPOPTIONS options);
int			GetProperties(LPCOMMANDPARAMS params, LPOPTIONS options, TCHAR*** properties);
LPOUTBUFFER ConvertTo(LPCOMMANDPARAMS params, LPOPTIONS options);
void		Extract(LPCOMMANDPARAMS params, LPOPTIONS options);

// called by installer. These should ideally be in the main dll.
extern "C" __declspec(dllexport) UINT __stdcall RefreshIcons(MSIHANDLE hInstall);
extern "C" __declspec(dllexport) UINT __stdcall LaunchGettingStarted(MSIHANDLE hInstall);

// private helper methods
void SetNameValue(TCHAR* name, TCHAR* value, TCHAR*** arr, int index);
int ReadBuffer(LPBUFFER buffer, ReturnObjectType* type);
void SetIndigoOptions(LPOPTIONS options);
