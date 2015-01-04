#pragma once

#include "stdafx.h"
#include "gtest\gtest.h"
#include <map>

class APITests : public ::testing::Test
{
protected:
	HINSTANCE m_dllHandle;
	std::map<ChemFormat, VERIFYDATA*> m_verifyDataMap;

public:
	APITests(void) : m_dllHandle(NULL)
	{
		TCHAR corePath[MAX_PATH];
		BOOL runTimeLinkSuccess = FALSE; 

		// make file path to provider dll in this module's directory
		GetModuleFileName(_AtlBaseModule.m_hInst, corePath, MAX_PATH);
		PathRemoveFileSpec(corePath);
		PathAppend(corePath, L"provider.dll");

		// load the dll and keep the handle to it
		CoInitialize(nullptr);
		m_dllHandle = LoadLibrary(corePath);
	
		// If the handle is valid, try to get the function addresses
		if (m_dllHandle != NULL) 
		{
			// Get pointer to our functions using GetProcAddress:
			pExecuteFunc = (ExecuteFuncType)GetProcAddress(m_dllHandle, "Execute");
		}

		// insert verify data against each format
		m_verifyDataMap.insert(std::make_pair(fmtMOLV2, &VerifyData[0]));
		m_verifyDataMap.insert(std::make_pair(fmtMOLV3, &VerifyData[1]));
		m_verifyDataMap.insert(std::make_pair(fmtRXNV2, &VerifyData[2]));
		m_verifyDataMap.insert(std::make_pair(fmtRXNV3, &VerifyData[3]));
		m_verifyDataMap.insert(std::make_pair(fmtINCHI, &VerifyData[4]));
		m_verifyDataMap.insert(std::make_pair(fmtCDXML, &VerifyData[5]));
		m_verifyDataMap.insert(std::make_pair(fmtEMF, &VerifyData[6]));
		m_verifyDataMap.insert(std::make_pair(fmtPNG, &VerifyData[7]));
		m_verifyDataMap.insert(std::make_pair(fmtPDF, &VerifyData[8]));
		m_verifyDataMap.insert(std::make_pair(fmtSVG, &VerifyData[9]));
		m_verifyDataMap.insert(std::make_pair(fmtMDLCT, &VerifyData[10]));
		m_verifyDataMap.insert(std::make_pair(fmtCML, &VerifyData[11]));
	}
	~APITests(void)
	{
		if(m_dllHandle != NULL)
			FreeLibrary(m_dllHandle);
	}

	virtual void SetUp() { }
	virtual void TearDown()	{ }

	void VERIFY_CONVERSION(enum ChemFormat targetFormats[], int count, LPBUFFER buffer)
	{
		OPTIONS options;

		for (int index = 0; index < count; index++)
		{
			ChemFormat targetFormat = targetFormats[index];

			TCHAR inFormat[MAX_FORMAT_LONG], outFormat[MAX_FORMAT_LONG];
			CommonUtils::GetFormatString(buffer->DataFormat, inFormat, MAX_FORMAT_LONG);
			CommonUtils::GetFormatString(targetFormat, outFormat, MAX_FORMAT_LONG);

			ASSERT_TRUE(pExecuteFunc != NULL) << "Execute Function pointer is NULL";
			COMMANDPARAMS params(cmdConvert, buffer, (LPVOID)targetFormat);
			std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));
			
			ASSERT_TRUE(outBuffer.get() != NULL) << "OutBuffer should never be NULL.";
			ASSERT_TRUE(outBuffer->pData != NULL) << inFormat << "==>>" << outFormat << " :OutBuffer Data is NULL. Conversion FAILED.";
			ASSERT_GT(outBuffer->DataLength, 0) << inFormat << "==>>" << outFormat << " :OutBuffer Length is ZERO. Conversion FAILED.";

			if(m_verifyDataMap.find(targetFormat) != m_verifyDataMap.end())
			{
				// we will only look in the first 512 bytes if data is too long
				std::string data((PCHAR)outBuffer->pData, (outBuffer->DataLength > 512) ? 512 : outBuffer->DataLength);
				std::string match(m_verifyDataMap[targetFormat]->data, strlen(m_verifyDataMap[targetFormat]->data));
			
				if(m_verifyDataMap[targetFormat]->startsWith)
				{
					ASSERT_TRUE(data.find(match) == 0) << inFormat << "==>>" << outFormat 
						<< " :Data does not START WITH verify data.\nVERIFYDATA=" << data.substr(0, 50);
				}
				else
				{
					ASSERT_TRUE(data.find(match) != std::string::npos) << inFormat << "==>>" << outFormat 
						<< " :Data does not CONTAIN verify data.\nVERIFYDATA=" << data.substr(0, 50);
				}
			}
		}
	}
};

TEST_F(APITests, CheckGetProperties)
{
	BUFFER buffer(false);
	OPTIONS options;
	TCHAR** props = NULL;

	int molCount = sizeof(InputMols)/sizeof(INPUT_MOLECULES);

	// test each input molecule
	for(int c = 0; c < molCount; c++)
	{
		buffer.pData = const_cast<PCHAR>(InputMols[c].mol);
		buffer.DataFormat = InputMols[c].molFormat;
		buffer.DataLength = strlen(InputMols[c].mol) + 1;

		ASSERT_TRUE(pExecuteFunc != NULL);

		// test with both search names and without it
		for(int j = 0; j < 2; j++)
		{
			COMMANDPARAMS params(cmdGetProperties, &buffer, (LPVOID)((j == 0) ? false : true));
			std::auto_ptr<OUTBUFFER> outBuffer(pExecuteFunc(&params, &options));

			props = (TCHAR**)outBuffer->pData;
			int propCount = (int)outBuffer->DataLength;

			if((buffer.DataFormat == fmtMOLV2) || (buffer.DataFormat == fmtMOLV3))
				EXPECT_EQ(18, propCount) << "MOL must have 18 properties. MolIndex=" << c;
			else if((buffer.DataFormat == fmtRXNV2) || (buffer.DataFormat == fmtRXNV3))
				EXPECT_EQ(4, propCount) << "RXN must have 4 properties. MolIndex=" << c;

			for(int i = 0; i < propCount; i++)
			{
				ASSERT_STRNE(props[2*i], NULL) << "Property Name cannot be NULL";
				ASSERT_STRNE(props[2*i], _T("")) << "Property Name cannot be empty";

				//EXPECT_STRNE(props[2*i + 1], NULL) << "Property Value is NULL for property: " << props[2*i] << ", MolIndex=" << c;
				//EXPECT_STRNE(props[2*i + 1], _T("")) << "Property Value is empty for property: " << props[2*i] << ", MolIndex=" << c;
			}
		}
	}
}

//TODO: Makr Draw API fully testable
//TEST_F(APITests, CheckDraw)
//{
//	BUFFER buffer;
//	OPTIONS options;
//	TCHAR** props = NULL;
//	RECT rect = { 0, 0, 100, 100 };
//
//	ASSERT_TRUE(pDrawFunc != NULL) << "Convert Function pointer is NULL";
//
//	buffer.pData = const_cast<PCHAR>(InputMols[8].mol);
//	buffer.DataFormat = InputMols[8].molFormat;
//	buffer.DataLength = strlen(InputMols[8].mol) + 1;
//
//	pDrawFunc(0, rect, &buffer, &options);
//}

TEST_F(APITests, CheckConvertToWithMOLV2)
{
	BUFFER buffer(false);
	buffer.pData = const_cast<PCHAR>(MOLV2_Benzene);
	buffer.DataFormat = fmtMOLV2;
	buffer.DataLength = strlen((PCHAR)buffer.pData) + 1;

	enum ChemFormat tryFormats[] = { fmtMOLV3, fmtINCHI, fmtCDXML, fmtEMF, fmtPNG, fmtPDF, fmtSVG, fmtMDLCT, fmtCML };
	VERIFY_CONVERSION(tryFormats, sizeof(tryFormats)/sizeof(ChemFormat), &buffer);
}

TEST_F(APITests, CheckConvertToWithMOLV2Glucose)
{
	BUFFER buffer(false);
	buffer.pData = const_cast<PCHAR>(MOLV2_Glucose);
	buffer.DataFormat = fmtMOLV2;
	buffer.DataLength = strlen((PCHAR)buffer.pData) + 1;

	enum ChemFormat tryFormats[] = { fmtMOLV3, fmtINCHI, fmtCDXML, fmtEMF, fmtPNG, fmtPDF, fmtSVG, fmtMDLCT, fmtCML };
	VERIFY_CONVERSION(tryFormats, sizeof(tryFormats)/sizeof(ChemFormat), &buffer);
}

TEST_F(APITests, CheckConvertToWithMOLV3)
{
	BUFFER buffer(false);
	buffer.pData = const_cast<PCHAR>(MOLV3_NoNose);
	buffer.DataFormat = fmtMOLV3;
	buffer.DataLength = strlen((PCHAR)buffer.pData) + 1;

	enum ChemFormat tryFormats[] = { fmtMOLV2, fmtINCHI, fmtCDXML, fmtEMF, fmtPNG, fmtPDF, fmtSVG, fmtMDLCT, fmtCML };
	VERIFY_CONVERSION(tryFormats, sizeof(tryFormats)/sizeof(ChemFormat), &buffer);
}

TEST_F(APITests, CheckConvertToWithRXNV2)
{
	BUFFER buffer(false);
	buffer.pData = const_cast<PCHAR>(RXNV2_Diels);
	buffer.DataFormat = fmtRXNV2;
	buffer.DataLength = strlen((PCHAR)buffer.pData) + 1;

	//TODO: Add fmtCML when Convert to CML issues are fixed
	enum ChemFormat tryFormats[] = { fmtRXNV3, fmtEMF, fmtPNG, fmtPDF, fmtSVG, fmtMDLCT };
	VERIFY_CONVERSION(tryFormats, sizeof(tryFormats)/sizeof(ChemFormat), &buffer);
}

TEST_F(APITests, CheckConvertToWithRXNV3)
{
	BUFFER buffer(false);
	buffer.pData = const_cast<PCHAR>(RXNV3_Amide);
	buffer.DataFormat = fmtRXNV3;
	buffer.DataLength = strlen((PCHAR)buffer.pData) + 1;

	//TODO: Add fmtCML when Convert to CML issues are fixed
	enum ChemFormat tryFormats[] = { fmtRXNV2, fmtEMF, fmtPNG, fmtPDF, fmtSVG, fmtMDLCT };
	VERIFY_CONVERSION(tryFormats, sizeof(tryFormats)/sizeof(ChemFormat), &buffer);
}

//TODO: Add tests for more formats
