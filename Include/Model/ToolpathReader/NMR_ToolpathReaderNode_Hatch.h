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

--*/

#ifndef __NMR_TOOLPATHREADERNODE_HATCH
#define __NMR_TOOLPATHREADERNODE_HATCH

#include "Model/Reader/NMR_ModelReaderNode.h"
#include "Model/Reader/NMR_ModelReaderWarnings.h"
#include "Model/Classes/NMR_ModelToolpathLayerReadData.h"

namespace NMR {

	class CToolpathReaderNode_Hatch : public CModelReaderNode {
	protected:

		nfDouble m_dX1;
		nfDouble m_dY1;
		nfDouble m_dX2;
		nfDouble m_dY2;

		nfBool m_bHasX1;
		nfBool m_bHasY1;
		nfBool m_bHasX2;
		nfBool m_bHasY2;

		CModelToolpathLayerReadData * m_pReadData;

		virtual void OnNSChildElement(_In_z_ const nfChar * pChildName, _In_z_ const nfChar * pNameSpace, _In_ CXmlReader * pXMLReader);
		virtual void OnAttribute(_In_z_ const nfChar * pAttributeName, _In_z_ const nfChar * pAttributeValue);
		virtual void OnNSAttribute(_In_z_ const nfChar * pAttributeName, _In_z_ const nfChar * pAttributeValue, _In_z_ const nfChar * pNameSpace);

	public:
		CToolpathReaderNode_Hatch() = delete;
		CToolpathReaderNode_Hatch(_In_ PModelReaderWarnings pWarnings, _In_ PProgressMonitor pProgressMonitor, CModelToolpathLayerReadData * pReadData);

		virtual void parseXML(_In_ CXmlReader * pXMLReader);

		nfDouble getX1();
		nfDouble getY1();
		nfDouble getX2();
		nfDouble getY2();
	};

	typedef std::shared_ptr <CToolpathReaderNode_Hatch> PToolpathReaderNode_Hatch;

}

#endif // __NMR_TOOLPATHREADERNODE_HATCH

