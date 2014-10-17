// IndigoProvider.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "IndigoProvider.h"

#include <sys/stat.h>
#include "api\indigo.h"
#include "api\plugins\inchi\indigo-inchi.h"
#include "plugins\renderer\indigo-renderer.h"

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
			bool isSDF = ((buffer->DataFormat == fmtSDFV2) || (buffer->DataFormat == fmtSDFV3));
			bool isRDF = ((buffer->DataFormat == fmtRDFV2) || (buffer->DataFormat == fmtRDFV3));

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

				while((mol = indigoNext(ptr)) > 0)
				{
					bool isValid = false;

					// check if mol/reaction is valid
					if(isRDF) isValid = (indigoCountReactants(mol) > 0);
					else isValid = (indigoCountAtoms(mol) > 0);

					if(isValid)
					{
						index++;	// number of valid molecules in collection

						indigoArrayAdd(collection, mol);
						indigoFree(mol);

						// limit number of mol/reactions displayed depending on the file type
						if(isRDF && (index >= options->GridMaxReactions)) break;
						if((isSDF || (buffer->DataFormat == fmtCML)) && (index >= options->GridMaxMols)) break;
					}
				}

				if(index > 0)
				{
					indigoSetOptionInt("render-grid-title-font-size", 14);
					indigoSetOption("render-grid-title-property", "NAME");	// will display the 'name' property for mol, if it exists

					indigoSetOptionXY("render-grid-margins", 5, 5);
					//indigoSetOptionXY("render-image-size", rect.right - rect.left + 10, rect.bottom - rect.top + 10);

					indigoRenderGrid(collection, NULL, isRDF ? 1 : 2, dc);
				}
				else
				{
					// draw error bitmap (could be a different one for collection files)
					DrawErrorBitmap(hDC, &rect);
					return false;
				}

				indigoFree(collection);
			}

			if(options->IsThumbnail)
			{
				// draw a MOL version indicator on thumbnails for MOL, RXN, SDF and RDF
				if(CommonUtils::IsMOLV2000Format(buffer->DataFormat))
					DrawMOLVersionIndicator(hDC, true);
				else if(CommonUtils::IsMOLV3000Format(buffer->DataFormat))
					DrawMOLVersionIndicator(hDC, false);

				// draw approx record count value
				if((int)buffer->Extra > 0) DrawRecordCount(hDC, rect, (int)buffer->Extra);
			}
			else
			{
				size_t outlen;

				// return warnings only for previews
				const char* warning = indigoCheckBadValence(ptr);
				ALLOC_AND_COPY(warning, options->OutWarning1, &outlen);

				warning = indigoCheckAmbiguousH(ptr);
				ALLOC_AND_COPY(warning, options->OutWarning2, &outlen);
			}

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
		propCount = (int)FormatPropInfo[buffer->DataFormat][0];
		INT64 flags = FormatPropInfo[buffer->DataFormat][1];

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
				_snwprintf_s(temp, 500, 500, L"%s", 
					((buffer->DataFormat == fmtMOLV2) || (buffer->DataFormat == fmtRXNV2)) ? _T("V2000") 
						: (((buffer->DataFormat == fmtMOLV3) || (buffer->DataFormat == fmtRXNV3)) ? _T("V3000") : _T("NA")));
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

			if(flags & propSSSR)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountSSSR(mol));
				AddProperty(properties, index+=2, searchNames ? _T("sssr") : _T("SSSR"), temp);
			}
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

/**
	Converts the molecule specified through BUFFER to format specified in outFormat. 
	The method supports	the following formats:
	Input: MOL, RXN, SMILES, SMARTS, SDF, RDF, CML, SMILES
	Output: MOLV2000, MOLV3000, RXNV2000, RXNV3000, SMILES, INCHI, INCHIKEY, MDLCT, CML,
			CDXML, EMF, PDF, PNG, SVG
	In case of multimol files as input, it converts the first record in the file.
	@param buffer Input buffer specifies the file to convert
	@param outFormat Specified the output format
	@param options Specified the program options to use during convert
	@return Returns an instance of OUTBUFFER containing converted data and datalength values. The caller
			is responsible to delete the OUTBUFFER reference.
*/
INDIGOPROVIDER_API LPOUTBUFFER ConvertTo(LPBUFFER buffer, ChemFormat outFormat, LPOPTIONS options)
{
	USES_CONVERSION;

	LPOUTBUFFER outbuf = new OUTBUFFER();

	pantheios::log_NOTICE(_T("API-ConvertTo> Called. File="), buffer->FileName, 
			_T(", Length="), pantheios::integer(buffer->DataLength),
			_T(", OutFormat="), pantheios::integer(outFormat));

	char* retBuffer = NULL;
	ReturnObjectType retType = SingleMol;

	if(buffer->DataLength > 0)
	{
		int iter = 0;
		int ptr = ReadBuffer(buffer, &retType);

		// for multimol files, get the first record from reader
		if((ptr != -1) && (retType == MultiMol))
		{
			iter = ptr;		// in case of multimol, the return value is a iterator
			ptr = indigoNext(iter);
		}

		if(ptr != -1)
		{
			// in-memory buffer
			int bufHandle = NULL;

			switch(outFormat)
			{
				case fmtMOLV2:
				case fmtMOLV3:
					indigoSetOption("molfile-saving-mode", (outFormat == fmtMOLV2) ? "2000" : "3000");
					bufHandle = indigoWriteBuffer();
					if(indigoSaveMolfile(ptr, bufHandle) <= 0)
						pantheios::log_ERROR("API-ConvertTo> indigoSaveMolfile Failed. bufHandle=", pantheios::integer(bufHandle));
					break;

				case fmtRXNV2:
				case fmtRXNV3:
					indigoSetOption("molfile-saving-mode", (outFormat == fmtRXNV2) ? "2000" : "3000");
					bufHandle = indigoWriteBuffer();
					if(indigoSaveRxnfile(ptr, bufHandle) <= 0)
						pantheios::log_ERROR("API-ConvertTo> indigoSaveRxnfile Failed. bufHandle=", pantheios::integer(bufHandle));
					break;

				case fmtSMILES:
					{
						const char* smiles = indigoSmiles(ptr);
						ALLOC_AND_COPY(smiles, outbuf->pData, &outbuf->DataLength);
					}
					break;

				case fmtINCHI:
					{
						const char* inchi = indigoInchiGetInchi(ptr);
						ALLOC_AND_COPY(inchi, outbuf->pData, &outbuf->DataLength);
					}
					break;

				case fmtINCHIKEY:
					{
						//TODO: Crashing code
						const char* inchi = indigoInchiGetInchi(ptr);
						if(inchi != NULL)
						{
							const char* inchiKey = indigoInchiGetInchiKey(inchi);
							ALLOC_AND_COPY(inchiKey, outbuf->pData, &outbuf->DataLength);
						}
					}
					break;

				case fmtMDLCT:
					bufHandle = indigoWriteBuffer();
					if(indigoSaveMDLCT(ptr, bufHandle) <= 0)
						pantheios::log_ERROR("API-ConvertTo> indigoSaveMDLCT Failed. bufHandle=", pantheios::integer(bufHandle));
					break;
					
				case fmtCML:
					{
						if((buffer->DataFormat == fmtRXNV2) || (buffer->DataFormat == fmtRXNV3))
						{
							TCHAR format[MAX_FORMAT];
							CommonUtils::GetFormatString(outFormat, format, MAX_FORMAT);

							bufHandle = indigoWriteBuffer();
							int saver = indigoCreateSaver(bufHandle, W2A(format));
							indigoAppend(saver, ptr);
							indigoClose(saver);
						}
						else
						{
							const char* cml = indigoCml(ptr);
							ALLOC_AND_COPY(cml, outbuf->pData, &outbuf->DataLength);
						}
					}
					break;
					
				case fmtCDXML:
				case fmtEMF:
				case fmtPDF:
				case fmtPNG:
				case fmtSVG:
					{
						TCHAR format[MAX_FORMAT];
						CommonUtils::GetFormatString(outFormat, format, MAX_FORMAT);
						indigoSetOption("render-output-format", W2A(format));
						indigoSetOptionXY("render-image-size", options->RenderImageWidth, options->RenderImageHeight);

						// render file to buffer
						bufHandle = indigoWriteBuffer();
						indigoRender(ptr, bufHandle);
					}
					break;

				default:
					pantheios::log_ERROR("API-ConvertTo> Unsupported Format requested: ", pantheios::integer(outFormat));
					break;
			}

			// smiles, inchi, inchikey is directly copied to the return buffer
			if(bufHandle != NULL)
			{
				// get raw data from buffer and return it to the caller
				int outSize = 0;
				char* tempBuffer;
				if((indigoToBuffer(bufHandle, &tempBuffer, &outSize) > 0) && (outSize > 0))
				{
					outbuf->pData = new char[outSize];
					memcpy_s(outbuf->pData, outSize, tempBuffer, outSize);
					outbuf->DataLength = outSize;
				}
			}

			indigoFree(ptr);
			if(iter != 0) indigoFree(iter);
		}
		else
		{
			pantheios::log_NOTICE(_T("API-ConvertTo> ReadBuffer FAILED."));
		}
	}

	indigoFreeAllObjects();
	return outbuf;
}

INDIGOPROVIDER_API void Extract(LPEXTRACTPARAMS params, LPOPTIONS options)
{
	USES_CONVERSION;

	char* sourceFile = W2A(params->sourceFile);
	char* fileFormat = W2A(params->fileFormat);
	char* folderPath = W2A(params->folderPath);	// folderPath contains {DIR}\{FILENAME}%d.{EXT}

	int reader = -1;

	bool isSDF = ((params->sourceFormat == fmtSDFV2) || (params->sourceFormat == fmtSDFV3));
	bool isRDF = ((params->sourceFormat == fmtRDFV2) || (params->sourceFormat == fmtRDFV3));

	if(isSDF) reader = indigoIterateSDFile(sourceFile);
	else if(isRDF) reader = indigoIterateRDFile(sourceFile);
	else if(params->sourceFormat == fmtCML) reader = indigoIterateCMLFile(sourceFile);
	else if(params->sourceFormat == fmtSMILES) reader = indigoIterateSmilesFile(sourceFile);
	else
	{
		pantheios::log_ERROR(_T("API-Extract> Source file format is NOT supported. Src File="), params->sourceFile);
		return;
	}

	struct stat st;
	bool cancel = false;
	CallbackEventArgs args;
	int mol, goodIndex = 1, index = 0;
	int skipped = 0, noPerm = 0, writeFail = 0;
	char fullFilePath[MAX_PATH];
	
	args.type = progressWorking;
	while((mol = indigoNext(reader)) > 0)
	{
		index++;	// processed (includes skipped ones)

		// construct file name 
		_snprintf_s(fullFilePath, MAX_PATH, folderPath, index);
		
		// check if a file already exists with the same name
		bool fileExists = (stat(fullFilePath, &st) == 0);
		
		if(fileExists)
		{
			if(params->overwriteFiles)
			{
				// check if we have write permissions on this file
				if((st.st_mode & S_IWRITE) == 0)
				{
					// skipped: no permission to write for this file
					noPerm++;
					indigoFree(mol);
					continue;
				}
			}
			else
			{
				// skipped: param does not allow overwriting
				skipped++;
				indigoFree(mol);
				continue;
			}
		}

		if(isSDF && ((params->exportFormat == fmtMOLV2) || (params->exportFormat == fmtMOLV3)))
		{			
			indigoSetOption("molfile-saving-mode", (params->exportFormat == fmtMOLV2) ? "2000" : "3000");
			if(indigoSaveMolfileToFile(mol, fullFilePath) < 1) writeFail++;
		}
		else if(isRDF && ((params->exportFormat == fmtRXNV2) || (params->exportFormat == fmtRXNV3)))
		{
			indigoSetOption("molfile-saving-mode", (params->exportFormat == fmtRXNV2) ? "2000" : "3000");
			if(indigoSaveRxnfileToFile(mol, fullFilePath) < 1) writeFail++;
		}
		else if((params->sourceFormat == fmtCML) && (params->exportFormat == fmtCML))
		{
			if(indigoSaveCmlToFile(mol, fullFilePath) < 1) writeFail++;
		}
		else if((params->sourceFormat == fmtSMILES) && (params->exportFormat == fmtSMILES))
		{
			int smiFile = indigoWriteFile(fullFilePath);
			if(indigoSmilesAppend(smiFile, mol) < 1) writeFail++;
		}
		else if((params->exportFormat == fmtCDXML) || (params->exportFormat == fmtEMF))
		{
			TCHAR format[MAX_FORMAT];
			CommonUtils::GetFormatString(params->exportFormat, format, MAX_FORMAT);
			indigoSetOption("render-output-format", W2A(format));
			indigoSetOptionXY("render-image-size", options->RenderImageWidth, options->RenderImageHeight);

			// render to file
			if(indigoRenderToFile(mol, fullFilePath) < 1)
				pantheios::log_ERROR(_T("API-Extract> indigoRenderToFile failed for: "), fullFilePath);
		}
		else
		{
			pantheios::log_WARNING(_T("API-Extract> Unsupported export formats. DataFormat="), 
				pantheios::integer(params->exportFormat));
		}

		indigoFree(mol);

		// break if limit is reached
		if(goodIndex++ == params->extractMolCount) break;

		// callback to notify parent
		if(params->callback != NULL)
		{
			args.processed = index;
			cancel = params->callback(params->caller, &args);
		}

		// check if user cancelled
		if(cancel) break;
	}

	TCHAR summary[512];
	TCHAR summaryTemplate[256];

	// load summary template from resource
	LoadString(hInstance, IDS_EXTRACT_SUMMARY, summaryTemplate, 256);

	// construct a summary for extract operation and sedn it to the caller using callback method
	if(_sntprintf_s(summary, 512, summaryTemplate, PathFindFileName(params->sourceFile),
		index, goodIndex - 1, skipped, noPerm, writeFail) > 0)
	{
		if(params->callback != NULL) 
		{
			args.type = progressDone;
			args.message = summary;
			args.total = args.processed = index;
			params->callback(params->caller, &args);
		}
	}

	indigoFree(reader);
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

	bool isSDF = ((buffer->DataFormat == fmtSDFV2) || (buffer->DataFormat == fmtSDFV3));
	bool isRDF = ((buffer->DataFormat == fmtRDFV2) || (buffer->DataFormat == fmtRDFV3));

	if((buffer->DataFormat == fmtMOLV2) || (buffer->DataFormat == fmtMOLV3))
		return indigoLoadMoleculeFromBuffer(buffer->pData, (int)buffer->DataLength);
	else if((buffer->DataFormat == fmtRXNV2) || (buffer->DataFormat == fmtRXNV3))
		return indigoLoadReactionFromBuffer(buffer->pData, (int)buffer->DataLength);
	else if(buffer->DataFormat == fmtSMARTS)
		return indigoLoadSmartsFromBuffer(buffer->pData, (int)buffer->DataLength);
	else if(isSDF || isRDF || (buffer->DataFormat == fmtCML) || (buffer->DataFormat == fmtSMILES))
	{
		*type = MultiMol;
		int reader = indigoLoadBuffer(buffer->pData, (int)buffer->DataLength);

		if(isSDF) return indigoIterateSDF(reader);
		if(isRDF) return indigoIterateRDF(reader);
		if(buffer->DataFormat == fmtCML) return indigoIterateCML(reader);
		if(buffer->DataFormat == fmtSMILES) return indigoIterateSmiles(reader);
	}
	else
	{
		pantheios::log_ERROR(_T("ReadBuffer> Invalid file type="), buffer->FileName);
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

/**
	Draws a small mark on specified HDC to identify MOLFILE version.
*/
void DrawMOLVersionIndicator(HDC hDC, bool v2000)
{
	HBRUSH bgBrush = ::CreateSolidBrush(RGB(224, 224, 224));
	HPEN oldPen = (HPEN)SelectObject(hDC, GetStockObject(NULL_PEN));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, bgBrush);
	HFONT oldFont = (HFONT)SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));

	// draw empty rectangle
	::Ellipse(hDC, 8, 8, 30, 30);
	::SetBkMode(hDC, TRANSPARENT);
	::TextOut(hDC, 12, 12, v2000 ? _T("V2") : _T("V3"), 2);

	// revert to old pens and brushes
	SelectObject(hDC, oldFont);
	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);

	// delete the newly created pen
	DeleteObject(bgBrush);
}

void DrawRecordCount(HDC hDC, RECT rect, int recordCount)
{
	SIZE szText;
	wchar_t text[10];
	int len = _snwprintf_s(text, 10, 10, L"%d±", recordCount);

	// create a large font to display molecule count
	//HFONT font = CreateFont(20, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
	//	ANTIALIASED_QUALITY, FF_DONTCARE, NULL);
	HFONT oldFont = (HFONT)SelectObject(hDC, GetStockObject(DEFAULT_GUI_FONT));

	// get width of text with currently selected font
	GetTextExtentPoint(hDC, text, len, &szText);

	// add margins to text
	szText.cx += 10;
	szText.cy += 5;

	int destWidth = (rect.right - rect.left);
	int destHeight = (rect.bottom - rect.top);
	
	HBRUSH bgBrush = ::CreateSolidBrush(RGB(224, 224, 224));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, bgBrush);

	//HPEN pen = CreatePen(PS_SOLID, 1, RGB(7, 203, 75));	// little darker green than the fill colour
	HPEN oldPen = (HPEN)SelectObject(hDC, GetStockObject(NULL_PEN));

	Rectangle(hDC, destWidth - szText.cx - 5, 8, destWidth - 5, szText.cy + 8);

	SetBkMode(hDC, TRANSPARENT);
	//SetTextColor(hDC, RGB(3, 82, 31));
	TextOut(hDC, destWidth-szText.cx, 10, text, len);

	SelectObject(hDC, oldFont);
	SelectObject(hDC, oldBrush);
	SelectObject(hDC, oldPen);

	//DeleteObject(pen);
	DeleteObject(bgBrush);
	//DeleteObject(font);
}
