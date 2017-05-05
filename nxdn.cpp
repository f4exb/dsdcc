///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "nxdn.h"
#include "dsd_decoder.h"

namespace DSDcc
{

DSDNXDN::DSDNXDN(DSDDecoder *dsdDecoder) :
		m_dsdDecoder(dsdDecoder),
		m_state(NXDNRDCHFrame),
		m_lichEvenParity(0),
		m_symbolIndex(0)
{
    memset(m_syncBuffer, 0, 11);
    memset(m_lichBuffer, 0, 8);
}

DSDNXDN::~DSDNXDN()
{
}

void DSDNXDN::init()
{
	m_symbolIndex = 0;
	m_lichEvenParity = 0;
	m_state = NXDNRDCHFrame;
}

void DSDNXDN::process()
{
    switch(m_state)
    {
    case NXDNRDCHFrame:
    	processRDCHFrame();
    	break;
    case NXDNRDCHPostFrame:
    	processRDCHPostFrame();
    	break;
    default:
        m_dsdDecoder->resetFrameSync(); // end
    }
}

void DSDNXDN::processRDCHFrame()
{
	int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

	if (m_symbolIndex < 8) // LICH info
	{
		if ((dibit == 0) || (dibit == 1))
		{ // positives => 0
			m_lichBuffer[m_symbolIndex] = 0;
		}
		else // negatives => 1
		{
			m_lichBuffer[m_symbolIndex] = 1;
			m_lichEvenParity++;
		}

		m_symbolIndex++;

		if (m_symbolIndex == 8)
		{
			processLICH();
		}
	}
	else if (m_symbolIndex < 8 + 174) // TODO: just pass for now
	{
		m_symbolIndex++;
	}
	else
	{
		m_state = NXDNRDCHPostFrame; // look for next RDCH or end
		m_symbolIndex = 0;
	}
}

void DSDNXDN::processRDCHPostFrame()
{
	if (m_symbolIndex < 10)
	{
		m_symbolIndex++;

		if (m_symbolIndex == 10)
		{
			m_syncBuffer[10] = '\0';

			if (m_dsdDecoder->getSyncType() == DSDDecoder::DSDSyncNXDNP)
			{
				 if (memcmp(m_dsdDecoder->m_dsdSymbol.getSyncDibitBack(10), DSDDecoder::m_syncNXDNRDCHFSW, 10) == 0) {
					 init();
				 } else {
					 m_dsdDecoder->resetFrameSync(); // end
				 }
			}
			else if (m_dsdDecoder->getSyncType() == DSDDecoder::DSDSyncNXDNN)
			{
				 if (memcmp(m_dsdDecoder->m_dsdSymbol.getSyncDibitBack(10), DSDDecoder::m_syncNXDNRDCHFSW, 10) == 0) {
					 init();
				 } else {
					 m_dsdDecoder->resetFrameSync(); // end
				 }
			}
			else
			{
				m_dsdDecoder->resetFrameSync(); // end
			}
		}
	}
	else // out of sync => terminate
	{
		m_dsdDecoder->resetFrameSync(); // end
	}
}

void DSDNXDN::processLICH()
{
	m_lich.rfChannelCode = 2*m_lichBuffer[0] + m_lichBuffer[1]; // MSB first
	m_lich.fnChannelCode = 2*m_lichBuffer[2] + m_lichBuffer[3];
	m_lich.optionCode    = 2*m_lichBuffer[4] + m_lichBuffer[5];
	m_lich.direction     = m_lichBuffer[6];
	m_lich.parity        = m_lichBuffer[7];

	if (m_lich.parity != (m_lichEvenParity % 2)) {
		std::cerr << "DSDNXDN::processLICH: LICH parity error" << std::endl;
	} else if (m_lich.rfChannelCode != 2) {
		std::cerr << "DSDNXDN::processLICH: wrong RF channel type for RDCH: " << m_lich.rfChannelCode << std::endl;
	}
}

} // namespace DSDcc

