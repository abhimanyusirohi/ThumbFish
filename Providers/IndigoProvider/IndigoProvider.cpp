// IndigoProvider.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "IndigoProvider.h"

#include "api\indigo.h"
#include "api\plugins\inchi\indigo-inchi.h"
#include "plugins\renderer\indigo-renderer.h"

const enum PropFlags {
	propName = 0x1, propNumAtoms = 0x2, propNumBonds = 0x4, propImplH = 0x8, propHeavyAtoms = 0x10, propGrossFormula = 0x20,
	propMolWeight = 0x40, propMostAbundantMass = 0x80, propMonoIsotopicMass = 0x100, propIsChiral = 0x200, propHasCoord = 0x400,
	propHasZCoord = 0x800, propSmiles = 0x1000, propCanonicalSmiles = 0x2000, propLayeredCode = 0x4000, propInChI = 0x8000, 
	propInChIKey = 0x10000, propDataVersion = 0x20000
};

INDIGOPROVIDER_API bool Draw(HDC hDC, RECT rect, LPBUFFER buffer, LPOPTIONS options)
{
	pantheios::log_NOTICE(_T("API-Draw> Called. File="), buffer->FileName, 
			_T(", Length="), pantheios::integer(buffer->DataLength));

	ReturnObjectType retType = SingleMol;

	if(buffer->DataLength > 0)
	{
		int ptr = ReadBuffer(buffer, &retType);
		if(ptr != -1)
		{
			SetIndigoOptions(options);
			indigoSetOptionXY("render-image-size", rect.right - rect.left, rect.bottom - rect.top);

			// required because the render-backgroundcolor options fails
			::FillRect(hDC, &rect, (HBRUSH)::GetStockObject(WHITE_BRUSH));

			int dc = indigoRenderWriteHDC((void*)hDC, 0);

			int renderErr = 0;
			if(retType == SingleMol)
			{
				renderErr = indigoRender(ptr, dc);
				pantheios::log_NOTICE(_T("API-Draw> Render returned="), pantheios::integer(renderErr));
			}
			else
			{
				int mol = 0, index = 0;
				int collection = indigoCreateArray();

				while(mol = indigoNext(ptr))
				{
					// check if mol/reaction is valid
					bool isValid = false;
					if(buffer->FileExtension == extRDF) isValid = (indigoCountReactants(mol) > 0);
					else if((buffer->FileExtension == extSDF) || (buffer->FileExtension == extCML))
						isValid = (indigoCountAtoms(mol) > 0);

					if(isValid)
					{
						index++;	// number of valid molecules in collection

						indigoArrayAdd(collection, mol);
						indigoFree(mol);

						// limit number of mol/reactions displayed depending on the file type
						if((buffer->FileExtension == extRDF) && (index >= options->GridMaxReactions)) break;
						if(((buffer->FileExtension == extSDF) || (buffer->FileExtension == extCML)) 
							&& (index >= options->GridMaxMols)) break;
					}
				}

				if(index > 0)
				{
					indigoSetOptionInt("render-grid-title-font-size", 14);
					indigoSetOption("render-grid-title-property", "NAME");	// will display the 'name' property for mol, if it exists

					indigoSetOptionXY("render-grid-margins", 5, 5);
					//indigoSetOptionXY("render-image-size", rect.right - rect.left + 10, rect.bottom - rect.top + 10);

					int nCols = (buffer->FileExtension == extRDF) ? 1 : 2;
					indigoRenderGrid(collection, NULL, nCols, dc);
				}
				else
				{
					// draw error bitmap (could be a different one for collection files)
					DrawErrorBitmap(hDC, &rect);
					return false;
				}

				indigoFree(collection);
			}

			// draw a V3000 indicator for thumbnails
			if(options->IsThumbnail && (buffer->DataVersion == 2))
				DrawVersionIndicator(hDC);

			indigoFree(ptr);
			return true;
		}
		else
		{
			pantheios::log_NOTICE(_T("API-Draw> ReadBuffer FAILED"));
		}
	}
	else
	{
		pantheios::log_NOTICE(_T("API-Draw> Buffer is EMPTY."));
	}

	// zero length, too large or invalid file format
	DrawErrorBitmap(hDC, &rect);

	return false;
}

INDIGOPROVIDER_API int GetProperties(LPBUFFER buffer, TCHAR*** properties, LPOPTIONS options, bool searchNames)
{
	pantheios::log_NOTICE(_T("API-GetProperties> Called. File="), buffer->FileName, 
			_T(", Length="), pantheios::integer(buffer->DataLength));

	int propCount = 0;

	ReturnObjectType retType = SingleMol;
	int mol = ReadBuffer(buffer, &retType);
	if(mol != -1)
	{
		// get properties to display for this file type
		INT64 flags = GetPropFlagsForFile(buffer->FileName, &propCount);

		if(propCount > 0)
		{
			pantheios::log_NOTICE(_T("API-GetProperties> Properties to be read="), pantheios::integer(propCount));

			*properties = new TCHAR*[propCount * 2];

			int index = -2;
			wchar_t temp[500];

			if(flags & propName)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoName(mol));
				AddProperty(properties, index+=2, searchNames ? _T("name") : _T("Name"), temp);
			}

			if(flags & propDataVersion)
			{
				int dv = buffer->DataVersion;
				_snwprintf_s(temp, 500, 500, L"%s", 
					(dv == 1) ? _T("V2000") : ((dv == 2) ? _T("V3000") : _T("NA")));
				AddProperty(properties, index+=2, searchNames ? _T("version") : _T("Version"), temp);
			}

			if(flags & propNumAtoms)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountAtoms(mol));
				AddProperty(properties, index+=2, searchNames ? _T("na") : _T("Num Atoms"), temp);
			}

			if(flags & propNumBonds)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountBonds(mol));
				AddProperty(properties, index+=2, searchNames ? _T("nb") : _T("Num Bonds"), temp);
			}

			if(flags & propImplH)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountImplicitHydrogens(mol));
				AddProperty(properties, index+=2, searchNames ? _T("imph") : _T("Implicit Hydrogens"), temp);
			}

			if(flags & propHeavyAtoms)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountHeavyAtoms(mol));
				AddProperty(properties, index+=2, searchNames ? _T("heavya") : _T("Heavy Atoms"), temp);
			}

			if(flags & propGrossFormula)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoToString(indigoGrossFormula(mol)));
				AddProperty(properties, index+=2, searchNames ? _T("formula") : _T("Gross Formula"), temp);
			}

			if(flags & propMolWeight)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMolecularWeight(mol));
				AddProperty(properties, index+=2, searchNames ? _T("mweight") : _T("Molecular Weight"), temp);
			}

			if(flags & propMostAbundantMass)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMostAbundantMass(mol));
				AddProperty(properties, index+=2, searchNames ? _T("mamass") : _T("Most Abundant Mass"), temp);
			}

			if(flags & propMonoIsotopicMass)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMonoisotopicMass(mol));
				AddProperty(properties, index+=2, searchNames ? _T("mimass") : _T("Mono Isotopic Mass"), temp);
			}

			if(flags & propIsChiral)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoIsChiral(mol) == 0) ? _T("No") : _T("Yes"));
				AddProperty(properties, index+=2, searchNames ? _T("chiral") : _T("Is Chiral"), temp);
			}

			if(flags & propHasCoord)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoHasCoord(mol) == 0) ? _T("No") : _T("Yes"));
				AddProperty(properties, index+=2, searchNames ? _T("hascoord") : _T("Has Coordinates"), temp);
			}

			if(flags & propHasZCoord)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoHasZCoord(mol) == 0) ? _T("No") : _T("Yes"));
				AddProperty(properties, index+=2, searchNames ? _T("haszcoord") : _T("Has Z Coordinates"), temp);
			}

			if(flags & propSmiles)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoSmiles(mol));
				AddProperty(properties, index+=2, searchNames ? _T("-") : _T("SMILES"), temp);
			}

			if(flags & propCanonicalSmiles)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoCanonicalSmiles(mol));
				AddProperty(properties, index+=2, searchNames ? _T("-") : _T("Canonical SMILES"), temp);
			}

			if(flags & propLayeredCode)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoLayeredCode(mol));
				AddProperty(properties, index+=2, searchNames ? _T("-") : _T("Layered Code"), temp);
			}

			//TODO: INCHI CALCULATION FAILS - malloc_dbg fails
			const char* inchi = indigoInchiGetInchi(mol);
			if(flags & propInChI)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", inchi);
				AddProperty(properties, index+=2, searchNames ? _T("-") : _T("InChi"), temp);
			}

			//_snwprintf(temp, 500, L"%hs", indigoInchiGetInchiKey(inchi));
			//AddProperty(properties, 24, _T("InChi Key"), temp);
		}
		else
		{
			pantheios::log_NOTICE(_T("API-GetProperties> No properties selected for this file type."));
		}

		indigoFree(mol);
	}
	else
	{
		pantheios::log_NOTICE(_T("API-GetProperties> ReadBuffer FAILED"));
	}

	return propCount;
}

/* ConvertTo
** Converts the molecule specified through BUFFER to other formats specified in OPTIONS
*/
INDIGOPROVIDER_API char* ConvertTo(LPBUFFER buffer, LPOPTIONS options)
{
	pantheios::log_NOTICE(_T("API-ConvertTo> Called. File="), buffer->FileName, 
			_T(", Length="), pantheios::integer(buffer->DataLength),
			_T(", OutExt="), options->RenderOutputExtension);

	char* retBuffer = NULL;
	ReturnObjectType retType = SingleMol;

	if(buffer->DataLength > 0)
	{
		int ptr = ReadBuffer(buffer, &retType);
		if(ptr != -1)
		{
			if(retType == SingleMol)
			{
				// in-memory buffer
				int bufHandle = NULL;

				if(strcmpi(options->RenderOutputExtension, "mol") == 0)
				{
					// connection table version
					indigoSetOption("molfile-saving-mode", (options->MOLSavingMode == 0) ? "auto" 
						: ((options->MOLSavingMode == 1) ? "2000" : "3000"));

					bufHandle = indigoWriteBuffer();
					indigoSaveMolfile(ptr, bufHandle);
				}
				else if(strcmpi(options->RenderOutputExtension, "smi") == 0)
				{
					const char* smiles = indigoSmiles(ptr);
					if(smiles != NULL)
					{
						size_t len = strlen(smiles) + 1;
						retBuffer = new char[len];
						strcpy_s(retBuffer, len, smiles);
						options->OutBufferSize = len;
					}
				}
				else if(strcmpi(options->RenderOutputExtension, "inchi") == 0)
				{
					const char* inchi = indigoInchiGetInchi(ptr);
					if(inchi != NULL)
					{
						size_t len = strlen(inchi) + 1;
						retBuffer = new char[len];
						strcpy_s(retBuffer, len, inchi);
						options->OutBufferSize = len;
					}
				}
				else if(strcmpi(options->RenderOutputExtension, "inchik") == 0)
				{
					//TODO: Crashing code
					//const char* inchi = indigoInchiGetInchi(ptr);
					//const char* inchiKey = indigoInchiGetInchiKey(inchi);
					//size_t len = strlen(inchiKey) + 1;
					//retBuffer = new char[len];
					//strcpy_s(retBuffer, len, inchiKey);
					//options->OutBufferSize = len;
				}
				else
				{
					indigoSetOption("render-output-format", options->RenderOutputExtension);
					indigoSetOptionXY("render-image-size", options->RenderImageWidth, options->RenderImageHeight);
					//TODO: Paint bg color if required because setting the bg color option freaks out renderer
					//indigoSetOptionColor("render-background-color",  GetRValue(options->RenderBackgroundColor), 
					//					GetGValue(options->RenderBackgroundColor), GetBValue(options->RenderBackgroundColor));

					// render file to buffer
					bufHandle = indigoWriteBuffer();
					indigoRender(ptr, bufHandle);
				}

				// smiles, inchi, inchikey is directly copied to the return buffer
				if(bufHandle != NULL)
				{
					// get raw data from buffer and return it to the caller
					int outSize = 0;
					char* tempBuffer;
					if((indigoToBuffer(bufHandle, &tempBuffer, &outSize) > 0) && (outSize > 0))
					{
						retBuffer = new char[outSize];
						memcpy_s(retBuffer, outSize, tempBuffer, outSize);
						options->OutBufferSize = outSize;
					}
				}

				indigoFree(ptr);
			}
			else
			{
				//TODO: Conversion of multimol files such as SDF etc
				pantheios::log_WARNING(_T("API-ConvertTo> Function called for MultiMol file. Not Implemented."));
			}
		}
		else
		{
			pantheios::log_NOTICE(_T("API-ConvertTo> ReadBuffer FAILED."));
		}
	}

	return retBuffer;
}

/** Refresh the icon cache to rebuilt the thumbnails */
extern "C" UINT __stdcall RefreshIcons(MSIHANDLE hInstall)
{
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return ERROR_SUCCESS;
}

/** Open the Getting Started online link in default browser */
extern "C" UINT __stdcall LaunchGettingStarted(MSIHANDLE hInstall)
{
	ShellExecute(NULL, _T("open"), _T("http://abhimanyusirohi.github.io/ThumbFish/#gettingstarted"), 
		NULL, NULL, SW_SHOWNORMAL);
	return ERROR_SUCCESS;
}

void AddProperty(TCHAR*** properties, int startIndex, TCHAR* name, TCHAR* value)
{
	size_t len = _tcslen(name) + 1;
	(*properties)[startIndex] = new TCHAR[len];
	_tcscpy_s((*properties)[startIndex], len, name);

	len = _tcslen(value) + 1;
	(*properties)[startIndex + 1] = new TCHAR[len];
	_tcscpy_s((*properties)[startIndex + 1], len, value);
}

int ReadBuffer(LPBUFFER buffer, ReturnObjectType* type)
{
	if((buffer == NULL) || (buffer->DataLength <= 0)) return -1;

	if((buffer->FileExtension == extMOL) || (buffer->FileExtension == extSMI)
		|| (buffer->FileExtension == extSMILES))
		return indigoLoadMoleculeFromBuffer(buffer->pData, buffer->DataLength);
	else if(buffer->FileExtension == extRXN)
		return indigoLoadReactionFromBuffer(buffer->pData, buffer->DataLength);
	else if(buffer->FileExtension == extSMARTS)
		return indigoLoadSmartsFromBuffer(buffer->pData, buffer->DataLength);
	else if((buffer->FileExtension == extSDF) || (buffer->FileExtension == extRDF) || (buffer->FileExtension == extCML))
	{
		*type = MultiMol;
		int reader = indigoLoadBuffer(buffer->pData, buffer->DataLength);

		if(buffer->FileExtension == extSDF) return indigoIterateSDF(reader);
		if(buffer->FileExtension == extRDF) return indigoIterateRDF(reader);
		if(buffer->FileExtension == extCML) return indigoIterateCML(reader);
	}
	else
	{
		pantheios::log_WARNING(_T("ReadBuffer> Invalid file type="), 
			pantheios::integer(buffer->FileExtension));
	}

	return -1;
}

void SetIndigoOptions(LPOPTIONS options)
{
	if(options == NULL || !options->Changed) return;

	// Issue #57: coloring disabled for thumbnails
	indigoSetOptionBool("render-coloring", (options->RenderColoring && !options->IsThumbnail) ? 1 : 0);
	indigoSetOptionXY("render-margins", options->RenderMarginX, options->RenderMarginY);
	indigoSetOptionFloat("render-relative-thickness", options->RenderRelativeThickness);

	indigoSetOptionBool("render-implicit-hydrogens-visible", options->RenderImplicitH ? 1 : 0);
	indigoSetOptionBool("render-atom-ids-visible", options->RenderShowAtomID ? 1 : 0);
	indigoSetOptionBool("render-bond-ids-visible", options->RenderShowBondID ? 1 : 0);
	indigoSetOptionBool("render-atom-bond-ids-from-one", options->RenderAtomBondIDFromOne ? 1 : 0);
		
	indigoSetOptionColor("render-base-color", GetRValue(options->RenderBaseColor), 
		GetGValue(options->RenderBaseColor), GetBValue(options->RenderBaseColor));

	// Enabling this causes a IsWindow assertion failure and redraw problems
	//indigoSetOptionColor("render-background-color", GetRValue(options->RenderBackgroundColor), 
	//	GetGValue(options->RenderBackgroundColor), GetBValue(options->RenderBackgroundColor));
		
	indigoSetOption("render-label-mode", (options->RenderLabelMode == 0) ? "terminal-hetero" 
		: ((options->RenderLabelMode == 1) ? "hetero" : "none"));
	indigoSetOption("render-stereo-style", (options->RenderStereoStyle == 0) ? "old" 
		: ((options->RenderStereoStyle == 1) ? "ext" : "none"));
}

INT64 GetPropFlagsForFile(const TCHAR* fileName, int* numProps)
{
	LPTSTR ext = ::PathFindExtension(fileName);

	*numProps = 0;
	if(_tcsicmp(ext, _T(".mol")) == 0)
	{
		*numProps = 17;
		return ULONG_MAX; // set all bits, show all properties
	}
	else if(_tcsicmp(ext, _T(".rxn")) == 0)
	{
		*numProps = 4;
		return (propName | propSmiles | propIsChiral | propDataVersion);
	}
	else if((_tcsicmp(ext, _T(".smi")) == 0) || (_tcsicmp(ext, _T(".smiles")) == 0))
	{
		*numProps = 7;
		return (propName | propNumAtoms | propNumBonds | propHeavyAtoms | propSmiles | propCanonicalSmiles | propLayeredCode);
	}
	else if(_tcsicmp(ext, _T(".smarts")) == 0)
	{
		*numProps = 6;
		return (propName | propNumAtoms | propNumBonds | propHeavyAtoms | propGrossFormula | propSmiles);
	}
	else if((_tcsicmp(ext, _T(".sdf")) == 0) || (_tcsicmp(ext, _T(".rdf")) == 0) || (_tcsicmp(ext, _T(".cml")) == 0))
		return 0;
	else
		return 0;	// do not display any property
}

void DrawErrorBitmap(HDC hDC, LPRECT lpRect)
{
	if(hInstance == NULL) return;

	// load bitmap from resource
	HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FISH));
	if(hBitmap != NULL)
	{
		BITMAP bm = {0};
		GetObject(hBitmap, sizeof(bm), &bm);
		LONG bmcx = bm.bmWidth;
		LONG bmcy = bm.bmHeight;

		HDC hdcMem = CreateCompatibleDC(hDC);
		HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);

		::FillRect(hDC, lpRect, (HBRUSH)::GetStockObject(WHITE_BRUSH));

		// center the error bitmap in area
		if(!BitBlt(hDC, lpRect->left + ((lpRect->right - lpRect->left) - bmcx)/2, 
			lpRect->top + ((lpRect->bottom - lpRect->top) - bmcy)/2, lpRect->right,
			lpRect->bottom, hdcMem, 0, 0, SRCCOPY))
		{
			pantheios::log_WARNING(_T("DrawErrorBitmap> BitBlt API FAILED. GetLastError="), 
				pantheios::integer(::GetLastError()));
		}

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);
		DeleteObject(hBitmap);
	}
	else
	{
		pantheios::log_WARNING(_T("DrawErrorBitmap> LoadBitmap FAILED."));
	}
}

void DrawVersionIndicator(HDC hDC)
{
	HBRUSH greenBrush = ::CreateSolidBrush(RGB(7, 203, 75));
	HPEN oldPen = (HPEN)SelectObject(hDC, GetStockObject(WHITE_PEN));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, greenBrush);

	// draw empty rectangle
	::Ellipse(hDC, 10, 10, 25, 25);

	// revert to old pens and brushes
	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);

	// delete the newly created pen
	DeleteObject(greenBrush);
}
