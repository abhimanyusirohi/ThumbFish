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
	propInChIKey = 0x10000
};

INDIGOPROVIDER_API bool Draw(HDC hDC, RECT rect, LPBUFFER buffer, LPOPTIONS options)
{
	ReturnObjectType retType = SingleMol;
	int ptr = ReadBuffer(buffer, &retType);
	if(ptr != -1)
	{
		SetIndigoOptions(options);
		indigoSetOptionXY("render-image-size", rect.right - rect.left, rect.bottom - rect.top);

		// required because the render-backgroundcolor options fails
		::FillRect(hDC, &rect, (HBRUSH)::GetStockObject(WHITE_BRUSH));

		int dc = indigoRenderWriteHDC((void*)hDC, 0);

		if(retType == SingleMol)
		{
			indigoRender(ptr, dc);
		}
		else
		{
			int mol = 0, index = 0;
			int collection = indigoCreateArray();

			while(mol = indigoNext(ptr))
			{
				index++;

				indigoArrayAdd(collection, mol);
				indigoFree(mol);

				// limit number of mol/reactions displayed depending on the file type
				if((buffer->FileExtension == extRDF) && (index >= options->GridMaxReactions)) break;
				if(((buffer->FileExtension == extSDF) || (buffer->FileExtension == extCML)) 
					&& (index >= options->GridMaxMols)) break;
			}

			indigoSetOptionInt("render-grid-title-font-size", 14);
			indigoSetOption("render-grid-title-property", "NAME");	// will display the 'name' property for mol, if it exists

			indigoSetOptionXY("render-grid-margins", 5, 5);
			//indigoSetOptionXY("render-image-size", rect.right - rect.left + 10, rect.bottom - rect.top + 10);

			int nCols = (buffer->FileExtension == extRDF) ? 1 : 2;
			indigoRenderGrid(collection, NULL, nCols, dc);

			indigoFree(collection);
		}

		indigoFree(ptr);
		return true;
	}
	else
	{
		//TODO: Cannot read file. Draw error bitmap
	}

	return false;
}

INDIGOPROVIDER_API int GetProperties(LPBUFFER buffer, TCHAR*** properties, LPOPTIONS options)
{
	int propCount = 0;

	ReturnObjectType retType = SingleMol;
	int mol = ReadBuffer(buffer, &retType);
	if(mol != -1)
	{
		// get properties to display for this file type
		INT64 flags = GetPropFlagsForFile(buffer->FileName, &propCount);

		if(propCount > 0)
		{
			*properties = new TCHAR*[propCount * 2];

			int index = -2;
			wchar_t temp[500];

			if(flags & propName)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoName(mol));
				AddProperty(properties, index+=2, _T("Name"), temp);
			}

			if(flags & propNumAtoms)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountAtoms(mol));
				AddProperty(properties, index+=2, _T("Num Atoms"), temp);
			}

			if(flags & propNumBonds)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountBonds(mol));
				AddProperty(properties, index+=2, _T("Num Bonds"), temp);
			}

			if(flags & propImplH)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountImplicitHydrogens(mol));
				AddProperty(properties, index+=2, _T("Implicit Hydrogens"), temp);
			}

			if(flags & propHeavyAtoms)
			{
				_snwprintf_s(temp, 500, 500, L"%d", indigoCountHeavyAtoms(mol));
				AddProperty(properties, index+=2, _T("Heavy Atoms"), temp);
			}

			if(flags & propGrossFormula)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoToString(indigoGrossFormula(mol)));
				AddProperty(properties, index+=2, _T("Gross Formula"), temp);
			}

			if(flags & propMolWeight)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMolecularWeight(mol));
				AddProperty(properties, index+=2, _T("Molecular Weight"), temp);
			}

			if(flags & propMostAbundantMass)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMostAbundantMass(mol));
				AddProperty(properties, index+=2, _T("Most Abundant Mass"), temp);
			}

			if(flags & propMonoIsotopicMass)
			{
				_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMonoisotopicMass(mol));
				AddProperty(properties, index+=2, _T("Mono Isotopic Mass"), temp);
			}

			if(flags & propIsChiral)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoIsChiral(mol) == 0) ? _T("No") : _T("Yes"));
				AddProperty(properties, index+=2, _T("Is Chiral"), temp);
			}

			if(flags & propHasCoord)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoHasCoord(mol) == 0) ? _T("No") : _T("Yes"));
				AddProperty(properties, index+=2, _T("Has Coordinates"), temp);
			}

			if(flags & propHasZCoord)
			{
				_snwprintf_s(temp, 500, 500, L"%s", (indigoHasZCoord(mol) == 0) ? _T("No") : _T("Yes"));
				AddProperty(properties, index+=2, _T("Has Z Coordinates"), temp);
			}

			if(flags & propSmiles)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoSmiles(mol));
				AddProperty(properties, index+=2, _T("SMILES"), temp);
			}

			if(flags & propCanonicalSmiles)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoCanonicalSmiles(mol));
				AddProperty(properties, index+=2, _T("Canonical SMILES"), temp);
			}

			if(flags & propLayeredCode)
			{
				_snwprintf_s(temp, 500, 500, L"%hs", indigoLayeredCode(mol));
				AddProperty(properties, index+=2, _T("Layered Code"), temp);
			}

			//TODO: INCHI CALCULATION FAILS - malloc_dbg fails
			//const char* inchi = indigoInchiGetInchi(mol);
			//_snwprintf_s(temp, 500, 500, L"%hs", inchi);
			//AddProperty(properties, 22, _T("InChi"), temp);

			//_snwprintf(temp, 500, L"%hs", indigoInchiGetInchiKey(inchi));
			//AddProperty(properties, 24, _T("InChi Key"), temp);
		}

		indigoFree(mol);
	}

	return propCount;
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

std::string GetData(LPBUFFER buffer)
{
	std::string cdxBytes;
	if(buffer->isStream)
	{
		IStream* stream = (IStream*)buffer->pData;

		// read IStream data into a char buffer
		ULONG read = 0;
		std::auto_ptr<char> temp(new char[buffer->DataLength]);
		if(stream->Read(temp.get(), buffer->DataLength, &read) == S_OK)
		{
			cdxBytes.assign(temp.get(), read);
		}
	}
	else
	{
		cdxBytes.assign((char*)buffer->pData, buffer->DataLength);
	}
	return cdxBytes;
}

int ReadBuffer(LPBUFFER buffer, ReturnObjectType* type)
{
	if((buffer == NULL) || (buffer->DataLength <= 0)) return -1;

	std::string cdxBytes = GetData(buffer);
	if(cdxBytes.size() > 0)
	{
		if((buffer->FileExtension == extMOL) || (buffer->FileExtension == extSMI)
			|| (buffer->FileExtension == extSMILES))
			return indigoLoadMoleculeFromString(cdxBytes.c_str());
		else if(buffer->FileExtension == extRXN)
			return indigoLoadReactionFromString(cdxBytes.c_str());
		else if(buffer->FileExtension == extSMARTS)
			return indigoLoadSmartsFromString(cdxBytes.c_str());
		else if((buffer->FileExtension == extSDF) || (buffer->FileExtension == extRDF) || (buffer->FileExtension == extCML))
		{
			*type = MultiMol;
			int reader = indigoLoadString(cdxBytes.c_str());

			if(buffer->FileExtension == extSDF) return indigoIterateSDF(reader);
			if(buffer->FileExtension == extRDF) return indigoIterateRDF(reader);
			if(buffer->FileExtension == extCML) return indigoIterateCML(reader);
		}
		else
			return -1;
	}

	return -1;
}

void SetIndigoOptions(LPOPTIONS options)
{
	if(options == NULL || !options->Changed) return;

	indigoSetOptionBool("render-coloring", options->RenderColoring ? 1 : 0);
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
		*numProps = 15;
		return ULONG_MAX; // set all bits, show all properties
	}
	else if(_tcsicmp(ext, _T(".rxn")) == 0)
	{
		*numProps = 3;
		return (propName | propSmiles | propIsChiral);
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