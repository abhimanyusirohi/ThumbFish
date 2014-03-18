// IndigoProvider.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "IndigoProvider.h"

#include "api\indigo.h"
#include "api\plugins\inchi\indigo-inchi.h"
#include "plugins\renderer\indigo-renderer.h"

INDIGOPROVIDER_API bool Draw(HDC hDC, RECT rect, LPBUFFER buffer, int* options)
{
	int mol = LoadMolecule(buffer);
	if(mol != -1)
	{
		indigoSetOptionBool("render-coloring", 1);
		indigoSetOptionBool("render-highlight-thickness-enabled", 1);
		indigoSetOptionXY("render-margins", 20, 20);
		indigoSetOptionXY("render-image-size", rect.right - rect.left, rect.bottom - rect.top);

		::FillRect(hDC, &rect, (HBRUSH)::GetStockObject(WHITE_BRUSH));

		int dc = indigoRenderWriteHDC((void*)hDC, 0);
		indigoRender(mol, dc);

		indigoFree(mol);
		return true;
	}

	return false;
}

INDIGOPROVIDER_API int GetProperties(LPBUFFER buffer, TCHAR*** properties, int* options)
{
	int propCount = 0;

	int mol = LoadMolecule(buffer);
	if(mol != -1)
	{	
		propCount = 15;
		*properties = new TCHAR*[propCount * 2];

		int index = -2;
		wchar_t temp[500];

		_snwprintf_s(temp, 500, 500, L"%d", indigoCountAtoms(mol));
		AddProperty(properties, index+=2, _T("Num Atoms"), temp);

		_snwprintf_s(temp, 500, 500, L"%d", indigoCountBonds(mol));
		AddProperty(properties, index+=2, _T("Num Bonds"), temp);

		_snwprintf_s(temp, 500, 500, L"%d", indigoCountImplicitHydrogens(mol));
		AddProperty(properties, index+=2, _T("Implicit Hydrogens"), temp);

		_snwprintf_s(temp, 500, 500, L"%d", indigoCountHeavyAtoms(mol));
		AddProperty(properties, index+=2, _T("Heavy Atoms"), temp);

		_snwprintf_s(temp, 500, 500, L"%hs", indigoToString(indigoGrossFormula(mol)));
		AddProperty(properties, index+=2, _T("Gross Formula"), temp);

		_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMolecularWeight(mol));
		AddProperty(properties, index+=2, _T("Molecular Weight"), temp);

		_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMostAbundantMass(mol));
		AddProperty(properties, index+=2, _T("Most Abundant Mass"), temp);

		_snwprintf_s(temp, 500, 500, L"%f g/mol", indigoMonoisotopicMass(mol));
		AddProperty(properties, index+=2, _T("Mono Isotopic Mass"), temp);

		_snwprintf_s(temp, 500, 500, L"%d", indigoCountHeavyAtoms(mol));
		AddProperty(properties, index+=2, _T("Heavy Atom Count"), temp);

		_snwprintf_s(temp, 500, 500, L"%s", (indigoIsChiral(mol) == 0) ? _T("No") : _T("Yes"));
		AddProperty(properties, index+=2, _T("Is Chiral"), temp);

		_snwprintf_s(temp, 500, 500, L"%s", (indigoHasCoord(mol) == 0) ? _T("No") : _T("Yes"));
		AddProperty(properties, index+=2, _T("Has Coordinates"), temp);

		_snwprintf_s(temp, 500, 500, L"%s", (indigoHasZCoord(mol) == 0) ? _T("No") : _T("Yes"));
		AddProperty(properties, index+=2, _T("Has Z Coordinates"), temp);

		_snwprintf_s(temp, 500, 500, L"%hs", indigoSmiles(mol));
		AddProperty(properties, index+=2, _T("SMILES"), temp);

		_snwprintf_s(temp, 500, 500, L"%hs", indigoCanonicalSmiles(mol));
		AddProperty(properties, index+=2, _T("Canonical SMILES"), temp);

		_snwprintf_s(temp, 500, 500, L"%hs", indigoLayeredCode(mol));
		AddProperty(properties, index+=2, _T("Layered Code"), temp);

		//TODO: INCHI CALCULATION FAILS - malloc_dbg fails
		//const char* inchi = indigoInchiGetInchi(mol);
		//_snwprintf_s(temp, 500, 500, L"%hs", inchi);
		//AddProperty(properties, 22, _T("InChi"), temp);

		//_snwprintf(temp, 500, L"%hs", indigoInchiGetInchiKey(inchi));
		//AddProperty(properties, 24, _T("InChi Key"), temp);

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

int LoadMolecule(LPBUFFER buffer)
{
	if((buffer == NULL) || (buffer->DataLength <= 0)) return -1;

	std::string cdxBytes = GetData(buffer);
	if(cdxBytes.size() > 0)
	{
		LPTSTR ext = ::PathFindExtension(buffer->FileName);

		//TODO: Add CML
		if(StrCmpI(ext, _T(".smarts")) == 0)
			return indigoLoadSmartsFromString(cdxBytes.c_str());
		else if((StrCmpI(ext, _T(".mol")) == 0) || (StrCmpI(ext, _T(".sdf")) == 0)
			|| (StrCmpI(ext, _T(".smi")) == 0) || (StrCmpI(ext, _T(".smiles")) == 0))
			return indigoLoadMoleculeFromString(cdxBytes.c_str());
		else if((StrCmpI(ext, _T(".rxn")) == 0) || (StrCmpI(ext, _T(".rdf")) == 0))
			return indigoLoadReactionFromString(cdxBytes.c_str());
		else
			return -1;
	}

	return -1;
}

// This is the constructor of a class that has been exported.
// see IndigoProvider.h for the class definition
//CIndigoProvider::CIndigoProvider()
//{
//	return;
//}
