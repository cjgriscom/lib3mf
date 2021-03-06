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

NMR_ModelToolpath.cpp implements the Model Toolpath Class.

--*/

#include "Model/Classes/NMR_ModelConstants.h"
#include "Model/Classes/NMR_ModelToolpath.h"
#include "Common/NMR_Exception.h"
#include "Common/NMR_StringUtils.h"
#include "Common/NMR_UUID.h"

#include <sstream>
#include <algorithm>

namespace NMR {


	CModelToolpath::CModelToolpath(_In_ const ModelResourceID sID, _In_ CModel * pModel, double dUnitFactor)
		: CModelResource(sID, pModel), m_dUnitFactor (dUnitFactor)
	{
	}

	PModelToolpath CModelToolpath::make(_In_ const ModelResourceID sID, _In_ CModel * pModel, double dUnitFactor)
	{	
		return std::make_shared<CModelToolpath> (sID, pModel, dUnitFactor);
	}

	PModelToolpathLayer CModelToolpath::addLayer(const std::string & sPath, nfUint32 nMaxZ)
	{
		auto pLayer = std::make_shared<CModelToolpathLayer>(sPath, nMaxZ);
		m_Layers.push_back(pLayer);
		return pLayer;
	}

	nfUint32 CModelToolpath::getLayerCount()
	{
		return (nfUint32)m_Layers.size();
	}

	PModelToolpathLayer CModelToolpath::getLayer(nfUint32 nIndex)
	{
		if (nIndex >= m_Layers.size())
			throw CNMRException(NMR_ERROR_INVALIDINDEX);
		return m_Layers[nIndex];
	}

	double CModelToolpath::getUnitFactor()
	{
		return m_dUnitFactor;
	}

	PModelToolpathProfile CModelToolpath::addProfile(const std::string & sName, nfDouble dLaserPower, nfDouble dLaserSpeed, nfDouble dLaserFocus, nfUint32 nLaserIndex)
	{
		CUUID newUUID;
		std::string sUUID = newUUID.toString();

		auto pProfile = std::make_shared<CModelToolpathProfile>(sUUID, sName, dLaserPower, dLaserSpeed, dLaserFocus, nLaserIndex);
		m_Profiles.push_back(pProfile);

		m_ProfileMap.insert(std::make_pair (sUUID, pProfile));

		return pProfile;
	
	}


	PModelToolpathProfile CModelToolpath::addExistingProfile(const std::string & sUUID, const std::string & sName, nfDouble dLaserPower, nfDouble dLaserSpeed, nfDouble dLaserFocus, nfUint32 nLaserIndex)
	{
		CUUID checkUUID (sUUID.c_str());

		auto pProfile = std::make_shared<CModelToolpathProfile>(checkUUID.toString (), sName, dLaserPower, dLaserSpeed, dLaserFocus, nLaserIndex);
		m_Profiles.push_back(pProfile);

		m_ProfileMap.insert(std::make_pair(sUUID, pProfile));

		return pProfile;

	}


	nfUint32 CModelToolpath::getProfileCount()
	{
		return (nfUint32)m_Profiles.size();
	}

	PModelToolpathProfile CModelToolpath::getProfile(nfUint32 nIndex)
	{
		if (nIndex >= m_Profiles.size())
			throw CNMRException(NMR_ERROR_INVALIDINDEX);
		return m_Profiles[nIndex];
	}

	PModelToolpathProfile CModelToolpath::getProfileByUUID(std::string sUUID)
	{
		auto iIterator = m_ProfileMap.find(sUUID);
		if (iIterator != m_ProfileMap.end()) {
			return iIterator->second;
		}

		return nullptr;
	}


}

