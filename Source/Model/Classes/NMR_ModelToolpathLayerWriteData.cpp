/*++

Copyright (C) 2018 3MF Consortium

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Abstract:

NMR_ModelToolpathLayer.cpp defines the Model Toolpath Layer.

--*/

#include "Model/Classes/NMR_ModelToolpathLayerWriteData.h"
#include "Model/Classes/NMR_ModelConstants.h"
#include "Model/Classes/NMR_ModelObject.h"

#include "Common/NMR_Exception.h"
#include "Common/NMR_StringUtils.h"
#include "Common/Platform/NMR_XmlWriter_Native.h"
#include "Common/Platform/NMR_ImportStream_Shared_Memory.h"

#include "Common/NMR_UUID.h"

#include <sstream>
#include <algorithm>

namespace NMR {

	CModelToolpathLayerWriteData::CModelToolpathLayerWriteData(_In_ CModelToolpath * pModelToolpath, _In_ NMR::PModelWriter_3MF pModelWriter, const std::string & sPackagePath)
		: m_pModelToolpath (pModelToolpath), m_pModelWriter (pModelWriter), m_sPackagePath (sPackagePath)
	{
		if (pModelToolpath == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		if (pModelWriter.get() == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);

		CUUID uuid;
		m_sUUID = uuid.toString();

		m_pExportStream = std::make_shared<NMR::CExportStreamMemory>();

		m_pXmlWriter = std::make_shared<NMR::CXmlWriter_Native>(m_pExportStream);
		m_pXmlWriter->WriteStartDocument();
		m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_LAYER, XML_3MF_NAMESPACE_TOOLPATHSPEC);
		m_pXmlWriter->WriteAttributeString(XML_3MF_ATTRIBUTE_XMLNS, XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, nullptr, XML_3MF_NAMESPACE_ZCOMPRESSION);

		m_bWritingHeader = true;
		m_bWritingData = false;
		m_bWritingFinished = false;

		m_nIDCounter = 1;

	}

	CModelToolpathLayerWriteData::~CModelToolpathLayerWriteData()
	{
		if (!m_bWritingFinished)
			finishWriting ();
	}

	nfUint32 CModelToolpathLayerWriteData::RegisterProfile(_In_ PModelToolpathProfile pProfile)
	{
		if (!m_bWritingHeader)
			throw CNMRException(NMR_ERROR_TOOLPATH_NOTWRITINGHEADER);

		unsigned int nNewID = m_nIDCounter;
		m_nIDCounter++;

		m_Profiles.insert(std::make_pair(nNewID, pProfile));

		return nNewID;
	}

	nfUint32 CModelToolpathLayerWriteData::RegisterPart(_In_ PModelObject pObject)
	{
		if (!m_bWritingHeader)
			throw CNMRException(NMR_ERROR_TOOLPATH_NOTWRITINGHEADER);

		unsigned int nNewID = m_nIDCounter;
		m_nIDCounter++;

		m_Parts.insert(std::make_pair(nNewID, pObject));

		return nNewID;
	}

	void CModelToolpathLayerWriteData::WriteHatchData(const nfUint32 nProfileID, const nfUint32 nPartID, const nfUint32 nHatchCount, const nfInt32 * pX1Buffer, const nfInt32 * pY1Buffer, const nfInt32 * pX2Buffer, const nfInt32 * pY2Buffer)
	{
		std::string sPath;
		NMR::CChunkedBinaryStreamWriter * pStreamWriter = getStreamWriter(sPath);
		if (pX1Buffer == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		if (pY1Buffer == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		if (pX2Buffer == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		if (pY2Buffer == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);

		if (m_bWritingHeader)
			finishHeader();
		if (!m_bWritingData)
			throw CNMRException(NMR_ERROR_TOOLPATH_NOTWRITINGDATA);

		std::string sPartID = std::to_string(nPartID);
		std::string sProfileID = std::to_string(nProfileID);

		m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_SEGMENT, nullptr);
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_TYPE, nullptr, XML_3MF_TOOLPATHTYPE_HATCH);
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_PROFILEID, nullptr, sProfileID.c_str());
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_PARTID, nullptr, sPartID.c_str());

		if (pStreamWriter != nullptr) {
			unsigned int binaryKeyX1 = pStreamWriter->addIntArray(pX1Buffer, nHatchCount, eptDeltaPredicition);
			unsigned int binaryKeyY1 = pStreamWriter->addIntArray(pY1Buffer, nHatchCount, eptDeltaPredicition);
			unsigned int binaryKeyX2 = pStreamWriter->addIntArray(pX2Buffer, nHatchCount, eptDeltaPredicition);
			unsigned int binaryKeyY2 = pStreamWriter->addIntArray(pY2Buffer, nHatchCount, eptDeltaPredicition);

			std::string sKeyX1 = std::to_string(binaryKeyX1);
			std::string sKeyY1 = std::to_string(binaryKeyY1);
			std::string sKeyX2 = std::to_string(binaryKeyX2);
			std::string sKeyY2 = std::to_string(binaryKeyY2);

			m_pXmlWriter->WriteStartElement(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHELEMENT_HATCH, nullptr);
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_X1, nullptr, sKeyX1.c_str());
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_Y1, nullptr, sKeyY1.c_str());
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_X2, nullptr, sKeyX2.c_str());
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_Y2, nullptr, sKeyY2.c_str());
			m_pXmlWriter->WriteEndElement();


		}
		else {


			// TODO: make fast!
			unsigned int nIndex;
			for (nIndex = 0; nIndex < nHatchCount; nIndex++) {
				std::string sX1 = std::to_string(pX1Buffer[nIndex]);
				std::string sY1 = std::to_string(pY1Buffer[nIndex]);
				std::string sX2 = std::to_string(pX2Buffer[nIndex]);
				std::string sY2 = std::to_string(pY2Buffer[nIndex]);

				m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_HATCH, nullptr);
				m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_X1, nullptr, sX1.c_str());
				m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_Y1, nullptr, sY1.c_str());
				m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_X2, nullptr, sX2.c_str());
				m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_Y2, nullptr, sY2.c_str());
				m_pXmlWriter->WriteEndElement();

			}

		}

		m_pXmlWriter->WriteFullEndElement();

	}

	void CModelToolpathLayerWriteData::WriteLoop(const nfUint32 nProfileID, const nfUint32 nPartID, const nfUint32 nPointCount, const nfInt32 * pXBuffer, const nfInt32 * pYBuffer)
	{
		if (pXBuffer == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		if (pYBuffer == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);

		std::string sPath;
		NMR::CChunkedBinaryStreamWriter * pStreamWriter = getStreamWriter(sPath);

		if (m_bWritingHeader)
			finishHeader();
		if (!m_bWritingData)
			throw CNMRException(NMR_ERROR_TOOLPATH_NOTWRITINGDATA);

		std::string sPartID = std::to_string(nPartID);
		std::string sProfileID = std::to_string(nProfileID);

		m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_SEGMENT, nullptr);
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_TYPE, nullptr, XML_3MF_TOOLPATHTYPE_LOOP);
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_PROFILEID, nullptr, sProfileID.c_str());
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_PARTID, nullptr, sPartID.c_str());

		if (pStreamWriter != nullptr) {
			unsigned int binaryKeyX = pStreamWriter->addIntArray(pXBuffer, nPointCount, eptDeltaPredicition);
			unsigned int binaryKeyY = pStreamWriter->addIntArray(pYBuffer, nPointCount, eptDeltaPredicition);

			std::string sKeyX = std::to_string(binaryKeyX);
			std::string sKeyY = std::to_string(binaryKeyY);

			m_pXmlWriter->WriteStartElement(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHELEMENT_POINT, nullptr);
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_X1, nullptr, sKeyX.c_str());
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_Y1, nullptr, sKeyY.c_str());
			m_pXmlWriter->WriteEndElement();

		}
		else {


			unsigned int nIndex;
			for (nIndex = 0; nIndex < nPointCount; nIndex++) {
				std::string sX = std::to_string(pXBuffer[nIndex]);
				std::string sY = std::to_string(pYBuffer[nIndex]);

				m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_POINT, nullptr);
				m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_X, nullptr, sX.c_str());
				m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_Y, nullptr, sY.c_str());
				m_pXmlWriter->WriteEndElement();
			}

		}

		m_pXmlWriter->WriteFullEndElement();

	}

	void CModelToolpathLayerWriteData::WritePolyline(const nfUint32 nProfileID, const nfUint32 nPartID, const nfUint32 nPointCount, const nfInt32 * pXBuffer, const nfInt32 * pYBuffer)
	{
		std::string sPath;

		if (pXBuffer == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		if (pYBuffer == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);

		NMR::CChunkedBinaryStreamWriter * pStreamWriter = getStreamWriter(sPath);
		if (m_bWritingHeader)
			finishHeader();
		if (!m_bWritingData)
			throw CNMRException(NMR_ERROR_TOOLPATH_NOTWRITINGDATA);

		std::string sPartID = std::to_string(nPartID);
		std::string sProfileID = std::to_string(nProfileID);

		m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_SEGMENT, nullptr);
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_TYPE, nullptr, XML_3MF_TOOLPATHTYPE_POLYLINE);
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_PROFILEID, nullptr, sProfileID.c_str());
		m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_PARTID, nullptr, sPartID.c_str());

		if (pStreamWriter != nullptr) {

			unsigned int binaryKeyX = pStreamWriter->addIntArray(pXBuffer, nPointCount, eptDeltaPredicition);
			unsigned int binaryKeyY = pStreamWriter->addIntArray(pYBuffer, nPointCount, eptDeltaPredicition);

			std::string sKeyX = std::to_string(binaryKeyX);
			std::string sKeyY = std::to_string(binaryKeyY);

			m_pXmlWriter->WriteStartElement(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHELEMENT_POINT, nullptr);
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_X1, nullptr, sKeyX.c_str());
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_Y1, nullptr, sKeyY.c_str());

			m_pXmlWriter->WriteEndElement();

		}
		else {

			unsigned int nIndex;
			for (nIndex = 0; nIndex < nPointCount; nIndex++) {
				std::string sX = std::to_string(pXBuffer[nIndex]);
				std::string sY = std::to_string(pYBuffer[nIndex]);

				m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_POINT, nullptr);
				m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_X, nullptr, sX.c_str());
				m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_Y, nullptr, sY.c_str());
				m_pXmlWriter->WriteEndElement();
			}

		}

		m_pXmlWriter->WriteFullEndElement();

	}

	void CModelToolpathLayerWriteData::finishHeader()
	{
		std::string sPath;

		NMR::CChunkedBinaryStreamWriter * pStreamWriter = getStreamWriter(sPath);

		if (!m_bWritingHeader)
			throw CNMRException(NMR_ERROR_TOOLPATH_NOTWRITINGHEADER);

		m_bWritingHeader = false;

		m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_PARTS, nullptr);
		for (auto iPart : m_Parts) {
			bool bHasUUID = false;

			m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_PART, nullptr);
			std::string sID = std::to_string(iPart.first);
			std::string sUUID = iPart.second->uuid()->toString();

			m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_ID, nullptr, sID.c_str());
			m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_UUID, nullptr, sUUID.c_str());
			m_pXmlWriter->WriteEndElement();
		}

		m_pXmlWriter->WriteFullEndElement();

		m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_PROFILES, nullptr);
		for (auto iProfile : m_Profiles) {
			m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_PROFILE, nullptr);
			std::string sID = std::to_string(iProfile.first);
			std::string sUUID = iProfile.second->getUUID();

			m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_ID, nullptr, sID.c_str());
			m_pXmlWriter->WriteAttributeString(nullptr, XML_3MF_TOOLPATHATTRIBUTE_UUID, nullptr, sUUID.c_str());
			m_pXmlWriter->WriteEndElement();
		}

		m_pXmlWriter->WriteFullEndElement();

		m_pXmlWriter->WriteStartElement(nullptr, XML_3MF_TOOLPATHELEMENT_SEGMENTS, nullptr);

		if (pStreamWriter != nullptr)
			m_pXmlWriter->WriteAttributeString(XML_3MF_NAMESPACEPREFIX_LZMACOMPRESSION, XML_3MF_TOOLPATHATTRIBUTE_BINARY, nullptr, sPath.c_str());

		m_bWritingData = true;


	}


	NMR::PImportStream CModelToolpathLayerWriteData::createStream()
	{
		if (m_bWritingHeader)
			finishHeader();
		if (!m_bWritingData)
			throw CNMRException(NMR_ERROR_TOOLPATH_NOTWRITINGDATA);

		m_bWritingData = false;
		if (m_bWritingFinished)
			throw CNMRException(NMR_ERROR_TOOLPATH_DATAHASBEENWRITTEN);

		m_bWritingFinished = true;

		m_pXmlWriter->WriteFullEndElement(); // segments

		m_pXmlWriter->WriteFullEndElement(); // layer
		m_pXmlWriter->WriteEndDocument();
		m_pXmlWriter->Flush();

		// TODO: Do not copy but use Pipe-based importexportstream!
		NMR::CImportStream_Shared_Memory pImportStream(m_pExportStream->getData(), m_pExportStream->getDataSize());
		return pImportStream.copyToMemory();

	}

	NMR::CChunkedBinaryStreamWriter * CModelToolpathLayerWriteData::getStreamWriter(std::string & sPath)
	{
	
		return m_pModelWriter->findBinaryStream (m_sUUID, sPath);

	}

	double CModelToolpathLayerWriteData::getUnits()
	{
		return m_pModelToolpath->getUnitFactor();
	}

	std::string CModelToolpathLayerWriteData::getUUID()
	{
		return m_sUUID;
	}

	void CModelToolpathLayerWriteData::finishWriting()
	{
		if (!m_bWritingFinished) {
			PImportStream pImportStream = createStream();
			m_pModelWriter->addAdditionalAttachment (m_sPackagePath, pImportStream, PACKAGE_TOOLPATH_RELATIONSHIP_TYPE);

		}
	}


}

