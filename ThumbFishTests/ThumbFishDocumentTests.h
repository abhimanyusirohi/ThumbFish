#pragma once

#include "stdafx.h"
#include "gtest\gtest.h"
#include "ThumbFishDocument.h"

static const char* MOL_Benzene = "Benzene\r\n  \r\n\r\n  6  6  0  0  0  0  0  0  0  0999 V2000\r\n   -0.7145    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\r\n   -0.7145   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\r\n    0.0000   -0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\r\n    0.7145   -0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\r\n    0.7145    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\r\n    0.0000    0.8250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\r\n  1  2  2  0      \r\n  2  3  1  0      \r\n  3  4  2  0      \r\n  4  5  1  0      \r\n  5  6  2  0      \r\n  6  1  1  0      \r\nM  END\r\n";
static const char* RXN_Amide = "$RXN V3000\r\nAmide Reaction V3000\r\n  \r\n\r\nM  V30 COUNTS 2 1\r\nM  V30 BEGIN REACTANT\r\nM  V30 BEGIN CTAB\r\nM  V30 COUNTS 5 4 0 0 0\r\nM  V30 BEGIN ATOM\r\nM  V30 1 C -0.714471 0.206250 0.000000 0\r\nM  V30 2 C 0.000000 -0.206250 0.000000 0\r\nM  V30 3 C -0.714471 1.031250 0.000000 0\r\nM  V30 4 O 0.714471 0.206250 0.000000 0\r\nM  V30 5 O 0.000000 -1.031250 0.000000 0\r\nM  V30 END ATOM\r\nM  V30 BEGIN BOND\r\nM  V30 1 1 1 2\r\nM  V30 2 1 1 3\r\nM  V30 3 1 2 4\r\nM  V30 4 2 2 5\r\nM  V30 END BOND\r\nM  V30 END CTAB\r\nM  V30 BEGIN CTAB\r\nM  V30 COUNTS 3 2 0 0 0\r\nM  V30 BEGIN ATOM\r\nM  V30 1 N -0.206250 -0.000001 0.000000 0\r\nM  V30 2 R2 0.206250 0.714471 0.000000 0\r\nM  V30 3 R1 0.206250 -0.714471 0.000000 0\r\nM  V30 END ATOM\r\nM  V30 BEGIN BOND\r\nM  V30 1 1 1 2\r\nM  V30 2 1 1 3\r\nM  V30 END BOND\r\nM  V30 END CTAB\r\nM  V30 END REACTANT\r\nM  V30 BEGIN PRODUCT\r\nM  V30 BEGIN CTAB\r\nM  V30 COUNTS 7 6 0 0 0\r\nM  V30 BEGIN ATOM\r\nM  V30 1 C -1.071707 0.206250 0.000000 0\r\nM  V30 2 C -0.357235 -0.206250 0.000000 0\r\nM  V30 3 C -1.071707 1.031250 0.000000 0\r\nM  V30 4 N 0.357236 0.206250 0.000000 0\r\nM  V30 5 O -0.357235 -1.031250 0.000000 0\r\nM  V30 6 R1 1.071707 -0.206249 0.000000 0\r\nM  V30 7 R2 0.357235 1.031250 0.000000 0\r\nM  V30 END ATOM\r\nM  V30 BEGIN BOND\r\nM  V30 1 1 1 2\r\nM  V30 2 1 1 3\r\nM  V30 3 1 2 4\r\nM  V30 4 2 2 5\r\nM  V30 5 1 4 6\r\nM  V30 6 1 4 7\r\nM  V30 END BOND\r\nM  V30 END CTAB\r\nM  V30 END PRODUCT\r\nM  END\r\n";

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
	// create a memory stream
	IStream* stream = ::SHCreateMemStream((BYTE*)MOL_Benzene, strlen(MOL_Benzene) + 1);
	ASSERT_TRUE(stream != NULL) << "Memory stream could not be created";
	ASSERT_HRESULT_SUCCEEDED(m_doc.LoadFromStream(stream, 0)) << "ThumbFishDocument::LoadFromStream() failed";
	
	// there is no filename in mem stream so format will be unknown
	ASSERT_EQ(fmtUnknown, m_doc.m_Buffer.DataFormat) << "Buffer DataFormat must be UnKnown (ZERO)";
	ASSERT_EQ(m_doc.m_Buffer.DataLength, 611) << "Buffer DataLength must be set to greater than ZERO";

	stream->Release();
}

TEST_F(ThumbFishDocumentTests, CheckLoadStreamWithMOL)
{
	size_t len = strlen(MOL_Benzene) + 1;

	// create a memory stream
	IStream* stream = ::SHCreateMemStream((BYTE*)MOL_Benzene, len);
	ASSERT_TRUE(stream != NULL) << "Memory stream could not be created";

	m_doc.m_Buffer.DataLength = len;
	m_doc.m_Buffer.DataFormat = fmtMOLV2;

	ASSERT_TRUE(m_doc.LoadStream(stream)) << "ThumbFishDocument::LoadStream() failed";
	ASSERT_EQ(fmtMOLV2, m_doc.m_Buffer.DataFormat) << "Buffer DataFormat must be MOLV2";
	
	stream->Release();
}

TEST_F(ThumbFishDocumentTests, CheckLoadStreamWithRXNV3000)
{
	size_t len = strlen(MOL_Benzene) + 1;

	// create a memory stream
	IStream* stream = ::SHCreateMemStream((BYTE*)RXN_Amide, strlen(RXN_Amide) + 1);
	ASSERT_TRUE(stream != NULL) << "Memory stream could not be created";

	m_doc.m_Buffer.DataLength = len;
	m_doc.m_Buffer.DataFormat = fmtRXNV2;

	ASSERT_TRUE(m_doc.LoadStream(stream)) << "ThumbFishDocument::LoadStream() failed";
	ASSERT_EQ(fmtRXNV3, m_doc.m_Buffer.DataFormat) << "Buffer DataFormat must be RXNV3";
	
	stream->Release();
}
