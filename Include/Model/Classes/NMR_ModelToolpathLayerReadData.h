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

NMR_ModelToolpath.h defines the Model Toolpath Layer Data.

--*/

#ifndef __NMR_MODELTOOLPATHLAYERREADDATA
#define __NMR_MODELTOOLPATHLAYERREADDATA

#include "Common/NMR_Types.h" 
#include "Common/NMR_PagedVector.h" 

#include "Model/Classes/NMR_ModelToolpath.h" 
#include "Model/Writer/NMR_ModelWriter.h" 
#include "Model/Writer/NMR_ModelWriter_3MF.h" 

#include <vector>
#include "Common/Platform/NMR_ImportStream.h"  
#include <memory>
#include <map>
#include <string>

namespace NMR {


	class CModelToolpath;
	typedef std::shared_ptr <CModelToolpath> PModelToolpath;

	enum class eModelToolpathSegmentType : nfUint32 {
		HatchSegment = 0,
		LoopSegment = 1,
		PolylineSegment = 2
	};

	typedef struct {
		eModelToolpathSegmentType m_eType;
		nfUint32 m_nProfileID;
		nfUint32 m_nPartID;
		nfUint32 m_nStartPoint;
		nfUint32 m_nPointCount;
	} TOOLPATHREADSEGMENT;


	class CModelToolpathLayerReadData {
	private:
		std::string m_sUUID;

		PModelToolpath m_pModelToolpath;

		CPagedVector<TOOLPATHREADSEGMENT> m_Segments;
		CPagedVector<NVEC2> m_Points;
		TOOLPATHREADSEGMENT * m_pCurrentSegment;

		std::map<uint32_t, std::string> m_UUIDMap;

		double m_dUnits;

	public:
		CModelToolpathLayerReadData() = delete;
		CModelToolpathLayerReadData(_In_ PModelToolpath pModelToolpath);

		~CModelToolpathLayerReadData();

		double getUnits();

		std::string getUUID();

		void beginSegment(eModelToolpathSegmentType eType, nfUint32 nProfileID, nfUint32 nPartID);
		void endSegment();
		void addPoint (nfFloat fX, nfFloat fY);

		nfUint32 getSegmentCount();
		void getSegmentInfo (nfUint32 nSegmentIndex, eModelToolpathSegmentType & eType, nfUint32 & nProfileID, nfUint32 & nPartID, nfUint32 & nPointCount);
		NVEC2 getSegmentPoint (nfUint32 nSegmentIndex, nfUint32 nPointIndex);

		void registerUUID (nfUint32 nID, std::string sUUID);
		std::string mapIDtoUUID(nfUint32 nID);

	};

	typedef std::shared_ptr <CModelToolpathLayerReadData> PModelToolpathLayerReadData;



}

#endif // __NMR_MODELTOOLPATHLAYERREADDATA

