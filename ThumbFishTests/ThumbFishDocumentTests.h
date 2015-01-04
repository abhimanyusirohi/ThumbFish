#pragma once

#include "stdafx.h"
#include "gtest\gtest.h"
#include "ThumbFishDocument.h"

class ThumbFishDocumentTests :	public ::testing::Test
{
protected:
	LARGE_INTEGER m_move;
	ThumbFishDocument m_doc;

public:
	ThumbFishDocumentTests(void) 
	{
		m_move.QuadPart = 0;	
	}

	~ThumbFishDocumentTests(void) {}

	virtual void SetUp() { }
	virtual void TearDown()	{ }
};

/** Tests Go Here */
TEST_F(ThumbFishDocumentTests, CheckLoadFromStream)
{
	int molCount = sizeof(InputMols)/sizeof(INPUT_MOLECULES);

	// test each input molecule
	for(int c = 0; c < molCount; c++)
	{
		size_t dataLen = strlen(InputMols[c].mol) + 1;

		// create a memory stream
		IStream* stream = ::SHCreateMemStream((BYTE*)InputMols[c].mol, (UINT)dataLen);
		ASSERT_TRUE(stream != NULL) << "Memory stream could not be created";
		ASSERT_HRESULT_SUCCEEDED(m_doc.LoadFromStream(stream, 0)) << "ThumbFishDocument::LoadFromStream() failed";
	
		// there is no filename in mem stream so format will be unknown
		ASSERT_EQ(fmtUnknown, m_doc.m_Buffer.DataFormat) << "Buffer DataFormat must be UnKnown (ZERO)";
		ASSERT_EQ(m_doc.m_Buffer.DataLength, dataLen) << "Buffer DataLength is set incorrectly";

		stream->Release();
	}
}

TEST_F(ThumbFishDocumentTests, CheckLoadStream)
{
	int molCount = sizeof(InputMols)/sizeof(INPUT_MOLECULES);

	// test each input molecule
	for(int c = 0; c < molCount; c++)
	{
		size_t len = strlen(InputMols[c].mol) + 1;

		// create a memory stream
		IStream* stream = ::SHCreateMemStream((BYTE*)InputMols[c].mol, (UINT)len);
		ASSERT_TRUE(stream != NULL) << "Memory stream could not be created";

		m_doc.m_Buffer.DataLength = len;
		m_doc.m_Buffer.DataFormat = InputMols[c].molFormat;

		ASSERT_TRUE(m_doc.LoadStream(stream) == TRUE) << "ThumbFishDocument::LoadStream() failed for MolIndex=" << c;
		ASSERT_EQ(InputMols[c].molFormat, m_doc.m_Buffer.DataFormat) << "Buffer DataFormat is incorrect";
	
		stream->Release();
	}
}
