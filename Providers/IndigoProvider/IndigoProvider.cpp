// IndigoProvider.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "IndigoProvider.h"

#include <sys/stat.h>
#include "api\indigo.h"
#include "api\plugins\inchi\indigo-inchi.h"
#include "plugins\renderer\indigo-renderer.h"

//-----------------------------------------------------------------------------
// <Execute>
// The one and only API method that can execute all supported commands.
//-----------------------------------------------------------------------------
INDIGOPROVIDER_API LPOUTBUFFER Execute(LPCOMMANDPARAMS params, LPOPTIONS options)
{
	LPOUTBUFFER outBuffer = new OUTBUFFER();	// is always returned
	Command cmdId = (Command)params->CommandID;

	switch(cmdId)
	{
		case cmdVersion:
			outBuffer->DataLength = strlen(VersionString) + 1;
			outBuffer->pData = new char[outBuffer->DataLength];
			strcpy_s((PCHAR)outBuffer->pData, outBuffer->DataLength, VersionString);
			break;

		case cmdDraw:
			outBuffer->pData = (PVOID)Draw(params, options);
			break;

		case cmdGetProperties:
			{
				TCHAR** properties = NULL;
				outBuffer->DataLength = GetProperties(params, options, &properties);
				outBuffer->pData = properties;
			}
			break;

		case cmdConvert:
			delete outBuffer;
			outBuffer = ConvertTo(params, options);
			break;

		case cmdExtract:
			Extract(params, options);
			break;

		case cmdAromatize:
		case cmdDearomatize:
		case cmdCleanup:
		case cmdNormalize:
		case cmdFoldHydrogens:
		case cmdUnfoldHydrogens:
		case cmdValidate:
			DeleteAndNull(outBuffer);
			outBuffer = Perform(params, options);
			break;

		case cmdBrowse:
			Browse(params, options);
			break;
	}

	indigoFreeAllObjects();
	return outBuffer;
}

//-----------------------------------------------------------------------------
// <Draw>
// Draws single and multiple molecules on specified Device Context. This method
// is responsible for drawing both thumbnails and previews.
//-----------------------------------------------------------------------------
bool Draw(LPCOMMANDPARAMS params, LPOPTIONS options)
{
	LPBUFFER buffer = params->Buffer;
	LPDRAWPARAMS drawParams = (LPDRAWPARAMS)params->Param;

	pantheios::log_NOTICE(_T("API-Draw> Called. Format="), buffer->DataFormat, 
			_T(", DataLength="), pantheios::integer(buffer->DataLength));

	ReturnObjectType retType = SingleMol;

	if(buffer->DataLength > 0)
	{
		int ptr = ReadBuffer(buffer, &retType);
		if(ptr != -1)
		{
			bool isSDF = ((buffer->DataFormat == fmtSDFV2) || (buffer->DataFormat == fmtSDFV3));
			bool isRDF = ((buffer->DataFormat == fmtRDFV2) || (buffer->DataFormat == fmtRDFV3));

			SetIndigoOptions(options);

			// extra calculations to adjust the structure after updating Cairo library
			int width = drawParams->targetRect.right - drawParams->targetRect.left;
			int height = drawParams->targetRect.bottom - drawParams->targetRect.top;
			width -= options->RenderMarginX;
			height -= options->RenderMarginY;
			indigoSetOptionXY("render-image-size", width, height);

			// required because the render-backgroundcolor options fails
			HBRUSH bgBrush = ::CreateSolidBrush(options->RenderBackgroundColor);
			::FillRect(drawParams->hDC, &drawParams->targetRect, bgBrush);
			DeleteObject(bgBrush);

			int dc = indigoRenderWriteHDC((void*)drawParams->hDC, 0);

			int renderErr = 0;
			if(retType == SingleMol)
			{
				renderErr = indigoRender(ptr, dc);
				pantheios::log_NOTICE(_T("API-Draw> Render returned="), pantheios::integer(renderErr));
			}
			else
			{
				// if there is only one molecule in the multimol file then treat it as single
				// molecule file otherwise a single molecule will be drawn in a grid, aligned left
				if(buffer->recordCount == 1)
				{
					int mol = indigoNext(ptr);
					if(mol > 0)
						renderErr = indigoRender(mol, dc);
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
						Utils::DrawErrorBitmap(drawParams->hDC, &drawParams->targetRect);
						return false;
					}

					indigoFree(collection);
				}
			}

			if(options->IsThumbnail)
			{
				// draw a MOL version indicator on thumbnails for MOL, RXN, SDF and RDF
				if(CommonUtils::IsMOLV2000Format(buffer->DataFormat))
					Utils::DrawMOLVersionIndicator(drawParams->hDC, true);
				else if(CommonUtils::IsMOLV3000Format(buffer->DataFormat))
					Utils::DrawMOLVersionIndicator(drawParams->hDC, false);

				// draw approx record count value
				if(buffer->recordCount > 1) 
					Utils::DrawRecordCount(drawParams->hDC, drawParams->targetRect, buffer->recordCount);
			}
			else
			{
				size_t outlen = 0;

				//TODO: Call Perform for indigoCheckBadValence
				// return warnings only for previews
				const char* warning = indigoCheckBadValence(ptr);
				ALLOC_AND_COPY(warning, options->OutWarning1, &outlen);

				//TODO: Call Perform for indigoCheckAmbiguousH
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
	Utils::DrawErrorBitmap(drawParams->hDC, &drawParams->targetRect);

	return false;
}

//-----------------------------------------------------------------------------
// <GetProperties>
// Compiles a name-value array of molecule properties.
//-----------------------------------------------------------------------------
int GetProperties(LPCOMMANDPARAMS params, LPOPTIONS options, TCHAR*** properties)
{
	LPBUFFER buffer = params->Buffer;

	// specifies whether to return property names to use in search
	bool returnSearchNames = ((int)params->Param == 1);

	pantheios::log_NOTICE(_T("API-GetProperties> Called. File Format="), buffer->DataFormat, 
			_T(", DataLength="), pantheios::integer(buffer->DataLength));

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
				SetNameValue(returnSearchNames ? _T("name") : _T("Name"), temp, properties, index+=2);
			}

			if(flags & propDataVersion)
			{
				_snwprintf_s(temp, 500, 500, L"%s", 
					((buffer->DataFormat == fmtMOLV2) || (buffer->DataFormat == fmtRXNV2)) ? _T("V2000") 
						: (((buffer->DataFormat == fmtMOLV3) || (buffer->DataFormat == fmtRXNV3)) ? _T("V3000") : _T("NA")));
				SetNameValue(returnSearchNames ? _T("version") : _T("Version"), temp, properties, index+=2);
			}

			if(flags & propNumAtoms)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountAtoms(mol));
				SetNameValue(returnSearchNames ? _T("na") : _T("Num Atoms"), temp, properties, index+=2);
			}

			if(flags & propNumBonds)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountBonds(mol));
				SetNameValue(returnSearchNames ? _T("nb") : _T("Num Bonds"), temp, properties, index+=2);
			}

			if(flags & propImplH)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountImplicitHydrogens(mol));
				SetNameValue(returnSearchNames ? _T("imph") : _T("Implicit Hydrogens"), temp, properties, index+=2);
			}

			if(flags & propHeavyAtoms)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountHeavyAtoms(mol));
				SetNameValue(returnSearchNames ? _T("heavya") : _T("Heavy Atoms"), temp, properties, index+=2);
			}

			if(flags & propGrossFormula)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoToString(indigoGrossFormula(mol)));
				SetNameValue(returnSearchNames ? _T("formula") : _T("Gross Formula"), temp, properties, index+=2);
			}

			if(flags & propMolWeight)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMolecularWeight(mol));
				SetNameValue(returnSearchNames ? _T("mweight") : _T("Molecular Weight"), temp, properties, index+=2);
			}

			if(flags & propMostAbundantMass)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMostAbundantMass(mol));
				SetNameValue(returnSearchNames ? _T("mamass") : _T("Most Abundant Mass"), temp, properties, index+=2);
			}

			if(flags & propMonoIsotopicMass)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMonoisotopicMass(mol));
				SetNameValue(returnSearchNames ? _T("mimass") : _T("Mono Isotopic Mass"), temp, properties, index+=2);
			}

			if(flags & propIsChiral)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoIsChiral(mol) == 0) ? _T("No") : _T("Yes"));
				SetNameValue(returnSearchNames ? _T("chiral") : _T("Is Chiral"), temp, properties, index+=2);
			}

			if(flags & propHasCoord)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoHasCoord(mol) == 0) ? _T("No") : _T("Yes"));
				SetNameValue(returnSearchNames ? _T("hascoord") : _T("Has Coordinates"), temp, properties, index+=2);
			}

			if(flags & propHasZCoord)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoHasZCoord(mol) == 0) ? _T("No") : _T("Yes"));
				SetNameValue(returnSearchNames ? _T("haszcoord") : _T("Has Z Coordinates"), temp, properties, index+=2);
			}

			if(flags & propSmiles)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoSmiles(mol));
				SetNameValue(returnSearchNames ? _T("-") : _T("SMILES"), temp, properties, index+=2);
			}

			if(flags & propCanonicalSmiles)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoCanonicalSmiles(mol));
				SetNameValue(returnSearchNames ? _T("-") : _T("Canonical SMILES"), temp, properties, index+=2);
			}

			if(flags & propLayeredCode)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoLayeredCode(mol));
				SetNameValue(returnSearchNames ? _T("-") : _T("Layered Code"), temp, properties, index+=2);
			}

			const char* pInchi = indigoInchiGetInchi(mol);
			std::string inchi = (pInchi == NULL) ? "" : pInchi;
			if(flags & propInChI)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", inchi.c_str());
				SetNameValue(returnSearchNames ? _T("-") : _T("InChi"), temp, properties, index+=2);
			}

			if (flags & propInChIKey)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", (inchi.length() == 0) ? "" : indigoInchiGetInchiKey(inchi.c_str()));
				SetNameValue(returnSearchNames ? _T("-") : _T("InChiKey"), temp, properties, index += 2);
			}

			if(flags & propSSSR)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountSSSR(mol));
				SetNameValue(returnSearchNames ? _T("sssr") : _T("SSSR"), temp, properties, index+=2);
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

//-----------------------------------------------------------------------------
// <ConvertTo>
// Converts a specified molecule to format specified in outFormat (Param). 
// The method supports	the following formats:
// Input: MOL, RXN, SMILES, SMARTS, SDF, RDF, CML, SMILES
// Output:	MOLV2000, MOLV3000, RXNV2000, RXNV3000, SMILES, INCHI, INCHIKEY, 
//			MDLCT, CML, CDXML, EMF, PDF, PNG, SVG
// In case of multimol files as input, it converts the first record in the file.
//-----------------------------------------------------------------------------
LPOUTBUFFER ConvertTo(LPCOMMANDPARAMS params, LPOPTIONS options)
{
	LPBUFFER buffer = params->Buffer;
	ChemFormat outFormat = (ChemFormat)(int)params->Param;

	USES_CONVERSION;
	pantheios::log_NOTICE(_T("API-ConvertTo> Called. FileFormat="), buffer->DataFormat, 
			_T(", Length="), pantheios::integer(buffer->DataLength),
			_T(", OutFormat="), pantheios::integer(outFormat));

	//WORKAROUND: To be removed when provider issues with RXN -> CML, CDXML are fixed
	// avoid converting to CML,CDXML if source is reaction to avoid crash
	if(((buffer->DataFormat == fmtRXNV2) || (buffer->DataFormat == fmtRXNV3)) 
		&& ((outFormat == fmtCML) || (outFormat == fmtCDXML)))
	{
		return NULL;
	}

	char* retBuffer = NULL;
	ReturnObjectType retType = SingleMol;
	LPOUTBUFFER outbuf = new OUTBUFFER();

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
						const char* pInchi = indigoInchiGetInchi(ptr);
						if(pInchi != NULL)
						{
							std::string inchi = pInchi;
							const char* inchiKey = indigoInchiGetInchiKey(inchi.c_str());
							if (inchiKey != NULL)
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

	// better delete an empty out buffer
	if(outbuf && (outbuf->DataLength <= 0))
		DeleteAndNull(outbuf);

	return outbuf;
}

//-----------------------------------------------------------------------------
// <Extract>
// Extracts molecules from a multimol file such as SDF, RDF, CML or SMI.
//-----------------------------------------------------------------------------
void Extract(LPCOMMANDPARAMS params, LPOPTIONS options)
{
	LPBUFFER buffer = params->Buffer;
	LPEXTRACTPARAMS exParams = (LPEXTRACTPARAMS)params->Param;

	USES_CONVERSION;

	char* sourceFile = W2A(exParams->sourceFile);
	char* fileFormat = W2A(exParams->fileFormat);
	char* folderPath = W2A(exParams->folderPath);	// folderPath contains {DIR}\{FILENAME}%d.{EXT}

	int reader = -1;

	bool isSDF = ((exParams->sourceFormat == fmtSDFV2) || (exParams->sourceFormat == fmtSDFV3));
	bool isRDF = ((exParams->sourceFormat == fmtRDFV2) || (exParams->sourceFormat == fmtRDFV3));

	if(isSDF) reader = indigoIterateSDFile(sourceFile);
	else if(isRDF) reader = indigoIterateRDFile(sourceFile);
	else if(exParams->sourceFormat == fmtCML) reader = indigoIterateCMLFile(sourceFile);
	else if(exParams->sourceFormat == fmtSMILES) reader = indigoIterateSmilesFile(sourceFile);
	else
	{
		pantheios::log_ERROR(_T("API-Extract> Source file format is NOT supported. Src File="), exParams->sourceFile);
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
			if(exParams->overwriteFiles)
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

		if(isSDF && ((exParams->exportFormat == fmtMOLV2) || (exParams->exportFormat == fmtMOLV3)))
		{			
			indigoSetOption("molfile-saving-mode", (exParams->exportFormat == fmtMOLV2) ? "2000" : "3000");
			if(indigoSaveMolfileToFile(mol, fullFilePath) < 1) writeFail++;
		}
		else if(isRDF && ((exParams->exportFormat == fmtRXNV2) || (exParams->exportFormat == fmtRXNV3)))
		{
			indigoSetOption("molfile-saving-mode", (exParams->exportFormat == fmtRXNV2) ? "2000" : "3000");
			if(indigoSaveRxnfileToFile(mol, fullFilePath) < 1) writeFail++;
		}
		else if((exParams->sourceFormat == fmtCML) && (exParams->exportFormat == fmtCML))
		{
			if(indigoSaveCmlToFile(mol, fullFilePath) < 1) writeFail++;
		}
		else if((exParams->sourceFormat == fmtSMILES) && (exParams->exportFormat == fmtSMILES))
		{
			int smiFile = indigoWriteFile(fullFilePath);
			if(indigoSmilesAppend(smiFile, mol) < 1) writeFail++;
		}
		else if((exParams->exportFormat == fmtCDXML) || (exParams->exportFormat == fmtEMF))
		{
			TCHAR format[MAX_FORMAT];
			CommonUtils::GetFormatString(exParams->exportFormat, format, MAX_FORMAT);
			indigoSetOption("render-output-format", W2A(format));
			indigoSetOptionXY("render-image-size", options->RenderImageWidth, options->RenderImageHeight);

			// render to file
			if(indigoRenderToFile(mol, fullFilePath) < 1)
				pantheios::log_ERROR(_T("API-Extract> indigoRenderToFile failed for: "), fullFilePath);
		}
		else
		{
			pantheios::log_WARNING(_T("API-Extract> Unsupported export formats. DataFormat="), 
				pantheios::integer(exParams->exportFormat));
		}

		indigoFree(mol);

		// break if limit is reached
		if(goodIndex++ == exParams->extractMolCount) break;

		// callback to notify parent
		if(exParams->callback != NULL)
		{
			args.processed = index;
			cancel = exParams->callback(exParams->caller, &args);
		}

		// check if user cancelled
		if(cancel) break;
	}

	TCHAR summary[512];
	TCHAR summaryTemplate[256];

	// load summary template from resource
	LoadString(hInstance, IDS_EXTRACT_SUMMARY, summaryTemplate, 256);

	// construct a summary for extract operation and sedn it to the caller using callback method
	if(_sntprintf_s(summary, 512, summaryTemplate, PathFindFileName(exParams->sourceFile),
		index, goodIndex - 1, skipped, noPerm, writeFail) > 0)
	{
		if(exParams->callback != NULL) 
		{
			args.type = progressDone;
			args.message = summary;
			args.total = args.processed = index;
			exParams->callback(exParams->caller, &args);
		}
	}

	indigoFree(reader);
}

//-----------------------------------------------------------------------------
// <Perform>
// Performs quick fix tasks such as aromatization/dearomatization, fold/unfold
// hydrogens, cleanup, normalization and validation of a structure.
//-----------------------------------------------------------------------------
LPOUTBUFFER Perform(LPCOMMANDPARAMS params, LPOPTIONS options)
{
	bool succeeded = true;
	LPOUTBUFFER outBuffer = new OUTBUFFER;
	LPBUFFER buffer = params->Buffer;
	ReturnObjectType retType = SingleMol;

	int mol = ReadBuffer(buffer, &retType);
	if(mol != -1)
	{
		switch(params->CommandID)
		{
			case cmdAromatize:
				succeeded = (indigoAromatize(mol) == 1);
				break;

			case cmdDearomatize:
				succeeded = (indigoDearomatize(mol) == 1);
				break;

			case cmdCleanup:
				succeeded = (indigoLayout(mol) == 0);
				break;

			case cmdNormalize:
				succeeded = (indigoNormalize(mol, "") == 1);
				break;

			case cmdFoldHydrogens:
				succeeded = (indigoFoldHydrogens(mol) == 1);
				break;

			case cmdUnfoldHydrogens:
				succeeded = (indigoUnfoldHydrogens(mol) == 1);
				break;

			case cmdValidate:
				{
					const char* warning1 = indigoCheckBadValence(mol);
					const char* warning2 = indigoCheckAmbiguousH(mol);

					// warnings can be NULL for some mols such as SMARTS and empty "" 
					// for valid mols without problems
					size_t warningSize = ((warning1 == NULL) ? 0 : strlen(warning1)) 
						+ ((warning2 == NULL) ? 0 : strlen(warning2)) + 1;

					if(warningSize > 1)
					{
						warningSize += 2; // add extra for CRLF
						outBuffer->pData = new char[warningSize];
						outBuffer->DataLength = warningSize;

						// join the warnings one after another with aline break
						int len = sprintf_s((PCHAR)outBuffer->pData, outBuffer->DataLength, "%s\r\n", warning1);
						sprintf_s((PCHAR)outBuffer->pData + len - 1, outBuffer->DataLength, "%s", warning2);
					}
				}
				break;
		}

		if(!succeeded)
		{
			pantheios::log_WARNING(_T("Perform> FAILED for Command: "), pantheios::integer(params->CommandID));
		}
		else if(params->CommandID != cmdValidate) // cmdValidate sets up its own out buffer
		{
			int bufHandle = indigoWriteBuffer();
			if((buffer->DataFormat == fmtMOLV2) || (buffer->DataFormat == fmtMOLV3) || (buffer->DataFormat == fmtSMARTS))
			{
				indigoSetOption("molfile-saving-mode", (buffer->DataFormat == fmtMOLV2) ? "2000" : "3000");
				if(indigoSaveMolfile(mol, bufHandle) < 1) 
					pantheios::log_WARNING(_T("Perform> indigoSaveMolfile FAILED."));
			}
			else if((buffer->DataFormat == fmtRXNV2) || (buffer->DataFormat == fmtRXNV3))
			{
				indigoSetOption("molfile-saving-mode", (buffer->DataFormat == fmtMOLV2) ? "2000" : "3000");
				if(indigoSaveRxnfile(mol, bufHandle) < 1)
					pantheios::log_WARNING(_T("Perform> indigoSaveRxnfile FAILED."));
			}

			if(bufHandle != NULL)
			{
				// get raw data from buffer and return it to the caller
				int outSize = 0;
				char* tempBuffer;
				if((indigoToBuffer(bufHandle, &tempBuffer, &outSize) > 0) && (outSize > 0))
				{
					outBuffer->pData = new char[outSize];
					memcpy_s(outBuffer->pData, outSize, tempBuffer, outSize);
					outBuffer->DataLength = outSize;
				}
			}
		}
	}
	else
	{
		pantheios::log_WARNING(_T("Perform> ReadBuffer FAILED."));
	}

	return outBuffer;
}

//-----------------------------------------------------------------------------
// <Browse>
// Allows browsing through records in a multimol file.
//-----------------------------------------------------------------------------
void Browse(LPCOMMANDPARAMS params, LPOPTIONS options)
{
	LPBUFFER buffer = params->Buffer;
	LPBROWSEPARAMS browseParams = (LPBROWSEPARAMS)params->Param;

	int reader = -1;

	bool isSDF = ((browseParams->SourceFormat == fmtSDFV2) || (browseParams->SourceFormat == fmtSDFV3));
	bool isRDF = ((browseParams->SourceFormat == fmtRDFV2) || (browseParams->SourceFormat == fmtRDFV3));

	if(isSDF) reader = indigoIterateSDFile(browseParams->SourceFile);
	else if(isRDF) reader = indigoIterateRDFile(browseParams->SourceFile);
	else if(browseParams->SourceFormat == fmtCML) reader = indigoIterateCMLFile(browseParams->SourceFile);
	else if(browseParams->SourceFormat == fmtSMILES) reader = indigoIterateSmilesFile(browseParams->SourceFile);
	else
	{
		pantheios::log_ERROR(_T("API-Extract> Source file format is NOT supported. Src File="), browseParams->SourceFile);
		return;
	}

	int mol = -1;
	bool cancel = false;
	while(((mol = indigoNext(reader)) > 0) && !cancel)
	{
		BrowseEventArgs e;
		e.MolData = new OUTBUFFER();
		e.DataFormat = browseParams->SourceFormat;

		// write each record to a buffer
		int bufHandle = indigoWriteBuffer();
		switch(browseParams->SourceFormat)
		{
			case fmtSDFV2:
			case fmtSDFV3:
				e.DataFormat = (browseParams->SourceFormat == fmtSDFV2) ? fmtMOLV2 : fmtMOLV3;
				indigoSetOption("molfile-saving-mode", (browseParams->SourceFormat == fmtSDFV2) ? "2000" : "3000");
				indigoSaveMolfile(mol, bufHandle);
				break;

			case fmtRDFV2:
			case fmtRDFV3:
				e.DataFormat = (browseParams->SourceFormat == fmtRDFV2) ? fmtRXNV2 : fmtRXNV3;
				indigoSetOption("molfile-saving-mode", (browseParams->SourceFormat == fmtRDFV2) ? "2000" : "3000");
				indigoSaveRxnfile(mol, bufHandle);
				break;

			case fmtCML:
				{
					const char* cml = indigoCml(mol);
					ALLOC_AND_COPY(cml, e.MolData->pData, &e.MolData->DataLength);
				}
				break;

			case fmtSMILES:
				{
					std::string smilesWithLF;
					smilesWithLF = indigoSmiles(mol);
					smilesWithLF.append(1, LF);
					ALLOC_AND_COPY(smilesWithLF.c_str(), e.MolData->pData, &e.MolData->DataLength);
				}
				break;
		}
		
		if (e.MolData != NULL)
		{
			// read buffer into an outbuffer object
			int outSize = 0;
			char* tempBuffer;
			if ((indigoToBuffer(bufHandle, &tempBuffer, &outSize) > 0) && (outSize > 0))
			{
				e.MolData->pData = new char[outSize];
				memcpy_s(e.MolData->pData, outSize, tempBuffer, outSize);
				e.MolData->DataLength = outSize;
			}
		}

		// get properties for this molecule
		if(isSDF || isRDF)
		{
			int prop;
			int propIter = indigoIterateProperties(mol);
			while((prop = indigoNext(propIter)) > 0)
			{
				const char* propName = indigoName(prop);
				const char* propValue = indigoGetProperty(mol, propName);
				
				size_t propNameSize = strlen(propName) + 1, propValueSize = strlen(propValue) + 1;

				wchar_t* szPropName = new wchar_t[propNameSize];
				wchar_t* szPropValue = new wchar_t[propValueSize];

				// convert from ASCII to WIDE string
				_snwprintf_s(szPropName, propNameSize, propNameSize, L"%hs", propName);
				_snwprintf_s(szPropValue, propValueSize, propValueSize, L"%hs", propValue);
				e.Properties.insert(std::make_pair(szPropName, szPropValue));
			}
		}

		// call back into the calling code with record data
		if(browseParams->callback != NULL)
		{
			cancel = !browseParams->callback(browseParams->caller, &e);
		}
	}
}

#pragma region Installer Methods

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

#pragma endregion

#pragma region Helper Methods

//-----------------------------------------------------------------------------
// <SetNameValue>
// Sets the specified [name] and [value] strings in the specified double 
// dimension array of strings at the specified index.
//-----------------------------------------------------------------------------
void SetNameValue(TCHAR* name, TCHAR* value, TCHAR*** arr, int index)
{
	size_t len = _tcslen(name) + 1;
	(*arr)[index] = new TCHAR[len];
	_tcscpy_s((*arr)[index], len, name);

	len = _tcslen(value) + 1;
	(*arr)[index + 1] = new TCHAR[len];
	_tcscpy_s((*arr)[index + 1], len, value);
}

//-----------------------------------------------------------------------------
// <ReadBuffer>
// Reads the data in specified BUFFER object and returns an Indigo specific 
// object pointer.
//-----------------------------------------------------------------------------
int ReadBuffer(LPBUFFER buffer, ReturnObjectType* type)
{
	if((buffer == NULL) || (buffer->DataLength <= 0)) return -1;

	ChemFormat format = buffer->DataFormat;
	const PCHAR data = (PCHAR)buffer->pData;

	bool isSDF = ((format == fmtSDFV2) || (format == fmtSDFV3));
	bool isRDF = ((format == fmtRDFV2) || (format == fmtRDFV3));

	if((format == fmtMOLV2) || (format == fmtMOLV3))
		return indigoLoadMoleculeFromBuffer(data, (int)buffer->DataLength);
	else if((format == fmtRXNV2) || (format == fmtRXNV3))
		return indigoLoadReactionFromBuffer(data, (int)buffer->DataLength);
	else if(format == fmtSMARTS)
		return indigoLoadSmartsFromBuffer(data, (int)buffer->DataLength);
	else if(format == fmtINCHI)
		return indigoInchiLoadMolecule(data);
	else if(isSDF || isRDF || (format == fmtCML) || (format == fmtSMILES))
	{
		*type = MultiMol;
		int reader = indigoLoadBuffer(data, (int)buffer->DataLength);

		if(isSDF) return indigoIterateSDF(reader);
		if(isRDF) return indigoIterateRDF(reader);
		if(format == fmtCML) return indigoIterateCML(reader);
		if(format == fmtSMILES) return indigoIterateSmiles(reader);
	}
	else
	{
		pantheios::log_ERROR(_T("ReadBuffer> Invalid File Format="), format);
	}

	return -1;
}

//-----------------------------------------------------------------------------
// <SetIndigoOptions>
// Sets Indigo options using the OPTIONS instance.
//-----------------------------------------------------------------------------
void SetIndigoOptions(LPOPTIONS options)
{
	if(options == NULL || !options->Changed) return;

	indigoSetOptionXY("render-hdc-offset", options->HDC_offset_X, options->HDC_offset_Y);

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

#pragma endregion
