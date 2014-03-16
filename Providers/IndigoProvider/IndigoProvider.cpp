// IndigoProvider.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "IndigoProvider.h"

//#include "molecule/molecule.h"

//#include "base_cpp/scanner.h"
//#include "molecule/sdf_loader.h"
//#include "molecule/molfile_loader.h"
//#include "molecule/molecule_arom.h"
//#include "molecule/molecule_fingerprint.h"
//#include "base_c/bitarray.h"

#include "api\indigo.h"
#include "api\plugins\inchi\indigo-inchi.h"

INDIGOPROVIDER_API bool Draw(HDC hDC, RECT rect, LPBUFFER buffer, int* options)
{
	::FillRect(hDC, &rect, (HBRUSH)::GetStockObject(GRAY_BRUSH));

	return true;
}

INDIGOPROVIDER_API int GetProperties(LPBUFFER buffer, TCHAR*** properties, int* options)
{
	int propCount = 0;

	if(buffer->DataLength <= 0) return 0;

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

	if(cdxBytes.size() > 0)
	{	
		int mol = indigoLoadMoleculeFromString(cdxBytes.c_str());
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


// This is the constructor of a class that has been exported.
// see IndigoProvider.h for the class definition
//CIndigoProvider::CIndigoProvider()
//{
//	return;
//}
