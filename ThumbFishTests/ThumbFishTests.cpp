// ThumbFishTests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CommonUtilsTests.h"
#include "ThumbFishDocumentTests.h"

int _tmain(int argc, _TCHAR* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	int retVal = RUN_ALL_TESTS();

	::getchar();	// pause
	return retVal;
}

