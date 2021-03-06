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

NMR_ModelToolpathLayerReadData.cpp defines the Model Toolpath Layer Read Data.

--*/

#include "Model/Classes/NMR_ModelToolpathLayerReadData.h"
#include "Model/Classes/NMR_ModelConstants.h"
#include "Model/Classes/NMR_ModelObject.h"

#include "Common/NMR_Exception.h"
#include "Common/NMR_StringUtils.h"

#include "Common/NMR_UUID.h"

#include <sstream>
#include <algorithm>

namespace NMR {

	CModelToolpathLayerReadData::CModelToolpathLayerReadData(_In_ PModelToolpath pModelToolpath)
		: m_pModelToolpath (pModelToolpath), m_pCurrentSegment(nullptr)
	{
		if (pModelToolpath.get() == nullptr)
			throw CNMRException(NMR_ERROR_INVALIDPARAM);
	}

	CModelToolpathLayerReadData::~CModelToolpathLayerReadData()
	{

	}

	double CModelToolpathLayerReadData::getUnits()
	{
		return m_dUnits;
	}

	std::string CModelToolpathLayerReadData::getUUID()
	{
		return m_sUUID;
	}

	void CModelToolpathLayerReadData::beginSegment(eModelToolpathSegmentType eType, nfUint32 nProfileID, nfUint32 nPartID)
	{
		if (m_pCurrentSegment != nullptr)
			throw CNMRException(NMR_ERROR_LAYERSEGMENTALREADYOPEN);

		m_pCurrentSegment = m_Segments.allocData();
		m_pCurrentSegment->m_eType = eType;
		m_pCurrentSegment->m_nPartID = nPartID;
		m_pCurrentSegment->m_nProfileID = nProfileID;
		m_pCurrentSegment->m_nStartPoint = m_Points.getCount ();
		m_pCurrentSegment->m_nPointCount = 0;

	}

	void CModelToolpathLayerReadData::endSegment()
	{
		if (m_pCurrentSegment == nullptr)
			throw CNMRException(NMR_ERROR_LAYERSEGMENTNOTOPEN);

		nfUint32 nCount = m_Points.getCount();

		__NMRASSERT(m_pCurrentSegment->m_nStartPoint <= nCount);

		m_pCurrentSegment->m_nPointCount = nCount - m_pCurrentSegment->m_nStartPoint;

		m_pCurrentSegment = nullptr;
	}

	void CModelToolpathLayerReadData::addPoint(nfFloat fX, nfFloat fY)
	{
		if (m_pCurrentSegment == nullptr)
			throw CNMRException(NMR_ERROR_LAYERSEGMENTNOTOPEN);

		NVEC2 * pVec = m_Points.allocData();
		pVec->m_values.x = fX;
		pVec->m_values.y = fY;
	}

	nfUint32 CModelToolpathLayerReadData::getSegmentCount()
	{
		return m_Segments.getCount();
	}

	void CModelToolpathLayerReadData::getSegmentInfo(nfUint32 nSegmentIndex, eModelToolpathSegmentType & eType, nfUint32 & nProfileID, nfUint32 & nPartID, nfUint32 & nPointCount)
	{
		TOOLPATHREADSEGMENT * pSegment = m_Segments.getData(nSegmentIndex);
		__NMRASSERT(pSegment != nullptr);
		eType = pSegment->m_eType;
		nPartID = pSegment->m_nPartID;
		nProfileID = pSegment->m_nProfileID;
		nPointCount = pSegment->m_nPointCount;
	}

	NVEC2 CModelToolpathLayerReadData::getSegmentPoint(nfUint32 nSegmentIndex, nfUint32 nPointIndex)
	{
		TOOLPATHREADSEGMENT * pSegment = m_Segments.getData(nSegmentIndex);
		__NMRASSERT(pSegment != nullptr);
		if (nPointIndex >= pSegment->m_nPointCount)
			throw CNMRException(NMR_ERROR_INVALIDINDEX);

		return m_Points.getDataRef(pSegment->m_nStartPoint + nPointIndex);

	}

	void CModelToolpathLayerReadData::registerUUID(nfUint32 nID, std::string sUUID)
	{
		auto iIter = m_UUIDMap.find(nID);
		if (iIter != m_UUIDMap.end())
			throw CNMRException(NMR_ERROR_DUPLICATEID);

		m_UUIDMap.insert(std::make_pair (nID, sUUID));
	}

	std::string CModelToolpathLayerReadData::mapIDtoUUID(nfUint32 nID)
	{
		auto iIter = m_UUIDMap.find(nID);
		if (iIter == m_UUIDMap.end())
			throw CNMRException(NMR_ERROR_MISSINGID);

		return iIter->second;
	}

}
