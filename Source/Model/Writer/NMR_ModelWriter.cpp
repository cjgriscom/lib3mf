/*++

Copyright (C) 2019 3MF Consortium

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

NMR_ModelWriter.cpp implements the Model Writer Class.
A model writer exports the in memory represenation into a model file.

--*/

#include "Model/Writer/NMR_ModelWriter.h" 

#include "Model/Classes/NMR_ModelConstants.h" 
#include "Common/Platform/NMR_XmlWriter.h" 
#include "Common/NMR_Exception.h" 
#include "Common/NMR_Exception_Windows.h" 

#include <sstream>

namespace NMR {

	const int MIN_DECIMAL_PRECISION = 1;
	const int MAX_DECIMAL_PRECISION = 16;

	CModelWriter::CModelWriter(_In_ PModel pModel, _In_ nfBool bAllowBinaryStreams):
		m_nDecimalPrecision(6), m_bAllowBinaryStreams(bAllowBinaryStreams)	{
		if (!pModel.get())
			throw CNMRException(NMR_ERROR_INVALIDPARAM);

		m_pModel = pModel;
		m_pProgressMonitor = std::make_shared<CProgressMonitor>();
	}

	CModelWriter::~CModelWriter()
	{
		unregisterBinaryStreams();
	}

	void CModelWriter::SetProgressCallback(Lib3MFProgressCallback callback, void* userData)
	{
		m_pProgressMonitor->SetProgressCallback(callback, userData);
	}

	void CModelWriter::SetDecimalPrecision(nfUint32 nDecimalPrecision)
	{
		if ((nDecimalPrecision < MIN_DECIMAL_PRECISION) || (nDecimalPrecision > MAX_DECIMAL_PRECISION))
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
		m_nDecimalPrecision = nDecimalPrecision;
	}

	nfUint32 CModelWriter::GetDecimalPrecision()
	{
		return m_nDecimalPrecision;
	}
	CModel * CModelWriter::getModel()
	{
		return m_pModel.get();
	}

	void CModelWriter::registerBinaryStream(const std::string &sPath, const std::string & sUUID, PChunkedBinaryStreamWriter pStreamWriter)
	{
		if (!m_bAllowBinaryStreams)
			throw CNMRException(NMR_ERROR_BINARYSTREAMSNOTALLOWED);

		auto iPathIter = m_BinaryWriterPathMap.find(sPath);
		if (iPathIter != m_BinaryWriterPathMap.end())
			throw CNMRException(NMR_ERROR_DUPLICATEBINARYSTREAMPATH);

		auto iUUIDIter = m_BinaryWriterUUIDMap.find(sUUID);
		if (iUUIDIter != m_BinaryWriterUUIDMap.end())
			throw CNMRException(NMR_ERROR_DUPLICATEBINARYSTREAMUUID);

		m_BinaryWriterUUIDMap.insert(std::make_pair(sUUID, std::make_pair (sPath, pStreamWriter)));
		m_BinaryWriterPathMap.insert(std::make_pair(sPath, sUUID));
	}

	void CModelWriter::unregisterBinaryStreams()
	{
		m_BinaryWriterPathMap.clear();
		m_BinaryWriterUUIDMap.clear();
		m_BinaryWriterAssignmentMap.clear();
	}

	void CModelWriter::assignBinaryStream(const std::string &InstanceUUID, const std::string & sBinaryStreamUUID)
	{
		auto iUUIDIter = m_BinaryWriterUUIDMap.find(sBinaryStreamUUID);
		if (iUUIDIter == m_BinaryWriterUUIDMap.end())
			throw CNMRException(NMR_ERROR_BINARYSTREAMNOTFOUND);


		if ((sBinaryStreamUUID.length ()) > 0) {
			m_BinaryWriterAssignmentMap.insert(std::make_pair(InstanceUUID, sBinaryStreamUUID));
		}
		else {
			m_BinaryWriterAssignmentMap.erase(InstanceUUID);
		}

	}

	CChunkedBinaryStreamWriter * CModelWriter::findBinaryStream(const std::string &InstanceUUID, std::string & Path)
	{
		auto iAssignIter = m_BinaryWriterAssignmentMap.find(InstanceUUID);
		if (iAssignIter != m_BinaryWriterAssignmentMap.end()) {
			auto iWriterIter = m_BinaryWriterUUIDMap.find (iAssignIter->second);
			if (iWriterIter != m_BinaryWriterUUIDMap.end()) {
				Path = iWriterIter->second.first;
				return iWriterIter->second.second.get();
			}
		}

		return nullptr;
	}
}

