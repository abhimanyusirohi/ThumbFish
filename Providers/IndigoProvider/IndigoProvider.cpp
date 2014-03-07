// IndigoProvider.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "IndigoProvider.h"

// This is an example of an exported variable
//INDIGOPROVIDER_API int nIndigoProvider=0;

// This is an example of an exported function.
INDIGOPROVIDER_API bool Draw(HDC hDC, RECT rect, LPBUFFER buffer, int* options)
{
	::FillRect(hDC, &rect, (HBRUSH)::GetStockObject(GRAY_BRUSH));

	return true;
}

INDIGOPROVIDER_API int GetProperties(LPBUFFER buffer, TCHAR*** properties, int* options)
{
	int propCount = 1;
	*properties = new TCHAR*[propCount * 2];

	(*properties)[0] = new TCHAR[10];
	_tcscpy((*properties)[0], _T("MolWeight"));
	(*properties)[1] = new TCHAR[10];
	_tcscpy((*properties)[1], _T("123.56 mg"));

	return propCount;
}

// This is the constructor of a class that has been exported.
// see IndigoProvider.h for the class definition
//CIndigoProvider::CIndigoProvider()
//{
//	return;
//}
