#pragma once

#include "stdafx.h"
#include "gtest\gtest.h"

class CommonUtilsTests : public testing::Test
{
public:
	CommonUtilsTests(void)	{ }
	~CommonUtilsTests(void)	{ }

	virtual void SetUp() { }
	virtual void TearDown()	{ }
};

/** Tests Go Here */
TEST_F(CommonUtilsTests, CheckGetFormatFromFileName)
{
	ASSERT_EQ(CommonUtils::GetFormatFromFileName(_T("testfile.mol")), fmtMOLV2) 
		<< ".mol should default to MOLV2";
	ASSERT_EQ(CommonUtils::GetFormatFromFileName(_T("testfile.rxn")), fmtRXNV2) 
		<< ".rxn should default to RXNV2";
	ASSERT_EQ(CommonUtils::GetFormatFromFileName(_T("testfile.sdf")), fmtSDFV2) 
		<< ".sdf should default to SDFV2";
	ASSERT_EQ(CommonUtils::GetFormatFromFileName(_T("testfile.rdf")), fmtRDFV2) 
		<< ".rdf should default to RDFV2";
}

TEST_F(CommonUtilsTests, CheckGetFormatExtension)
{
	TCHAR buffer[100];

	CommonUtils::GetFormatExtension(fmtMOLV2, buffer, 100);
	ASSERT_STREQ(_T(".mol"), buffer) << "fmtMOLV2 should default to .mol extension";
}
