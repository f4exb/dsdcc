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
		m_state(NXDNFrame),
		m_pn(0xe4), // TS 1A v0103 section 4.6
		m_lichEvenParity(0),
		m_symbolIndex(0),
		m_swallowCount(0)
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
	m_state = NXDNFrame;
}

void DSDNXDN::process()
{
    switch(m_state)
    {
    case NXDNFrame:
    	processFrame();
    	break;
    case NXDNPostFrame:
    	processPostFrame();
    	break;
    case NXDNSwallow:
        processSwallow();
        break;
    default:
        m_dsdDecoder->resetFrameSync(); // end
    }
}

void DSDNXDN::processFrame()
{
	int dibit = m_dsdDecoder->m_dsdSymbol.getDibit();       // get dibit from symbol
	dibit = m_pn.getBit(m_symbolIndex) ? dibit ^ 2 : dibit; // apply PN scrambling. Inverting symbol is a XOR by 2 on the dibit.

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
	else if (m_symbolIndex < 8 + 174 - 1) // TODO: just pass for now
	{
		m_symbolIndex++;
	}
	else
	{
		m_state = NXDNPostFrame; // look for next RDCH or end
		m_symbolIndex = 0;
	}
}

void DSDNXDN::processPostFrame()
{
	if (m_symbolIndex < 10)
	{
	    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

        if ((dibit == 0) || (dibit == 1)) { // positives => 1 (+3)
            m_syncBuffer[m_symbolIndex] = 1;
        } else { // negatives => 3 (-3)
            m_syncBuffer[m_symbolIndex] = 3;
        }

		m_symbolIndex++;

		if (m_symbolIndex == 10) {
		    processFSW();
		}
	}
	else // out of sync => terminate
	{
		m_dsdDecoder->resetFrameSync(); // end
	}
}

void DSDNXDN::processFSW()
{
    int match_late2 = 0; // count of FSW symbols matches late by 2 symbols
    int match_late1 = 0; // count of FSW symbols matches late by 1 symbol
    int match_spot  = 0; // count of FSW symbols matches on the spot
    int match_earl1 = 0; // count of FSW symbols matches early by 1 symbol
    int match_earl2 = 0; // count of FSW symbols matches early by 2 symbols

    const unsigned char *fsw;

    if (m_dsdDecoder->getSyncType() == DSDDecoder::DSDSyncNXDNP) {
        fsw = DSDDecoder::m_syncNXDNRDCHFSW;
    } else if (m_dsdDecoder->getSyncType() == DSDDecoder::DSDSyncNXDNN) {
        fsw = DSDDecoder::m_syncNXDNRDCHFSWInv;
    } else {
        m_dsdDecoder->resetFrameSync(); // end
        return;
    }

    for (int i = 0; i < 10; i++)
    {
        if (m_syncBuffer[i] ==  fsw[i]) {
            match_spot++;
        }
        if ((i < 7) && (m_syncBuffer[i] ==  fsw[i+2])) {
            match_late2++;
        }
        if ((i < 8) && (m_syncBuffer[i] ==  fsw[i+1])) {
            match_late1++;
        }
        if ((i > 0) && (m_syncBuffer[i] ==  fsw[i-1])) {
            match_earl1++;
        }
        if ((i > 1) && (m_syncBuffer[i] ==  fsw[i-2])) {
            match_earl2++;
        }
    }

    if (match_spot >= 7)
    {
        init();
    }
    else if (match_earl1 >= 6)
    {
        std::cerr << "DSDNXDN::processFSW: match early -1" << std::endl;
        m_swallowCount = 1;
        m_state = NXDNSwallow;
    }
    else if (match_late1 >= 6)
    {
        std::cerr << "DSDNXDN::processFSW: match late +1" << std::endl;
        m_lichBuffer[0] = m_syncBuffer[9] < 2 ? 0 : 1; // re-introduce last symbol (positive: 0, negative: 1) TODO: init LICH parity
        m_symbolIndex = 1;
        m_state = NXDNFrame;
    }
    else if (match_earl2 >= 5)
    {
        std::cerr << "DSDNXDN::processFSW: match early -2" << std::endl;
        m_swallowCount = 2;
        m_state = NXDNSwallow;
    }
    else if (match_late2 >= 5)
    {
        std::cerr << "DSDNXDN::processFSW: match late +2" << std::endl;
        m_lichBuffer[0] = m_syncBuffer[8] < 2 ? 0 : 1; // re-introduce last symbol (positive: 0, negative: 1) TODO: init LICH parity
        m_lichBuffer[1] = m_syncBuffer[9] < 2 ? 0 : 1; // re-introduce last symbol (positive: 0, negative: 1) TODO: init LICH parity
        m_symbolIndex = 2;
        m_state = NXDNFrame;
    }
    else
    {
        std::cerr << "DSDNXDN::processFSW: sync lost" << std::endl;
        m_dsdDecoder->resetFrameSync(); // end
    }
}

void DSDNXDN::processSwallow()
{
    if (m_swallowCount > 0) {
        m_swallowCount--;
    }

    if (m_swallowCount == 0) {
        init();
    }
}

void DSDNXDN::processLICH()
{
	m_lich.rfChannelCode = 2*m_lichBuffer[0] + m_lichBuffer[1]; // MSB first
	m_lich.fnChannelCode = 2*m_lichBuffer[2] + m_lichBuffer[3];
	m_lich.optionCode    = 2*m_lichBuffer[4] + m_lichBuffer[5];
	m_lich.direction     = m_lichBuffer[6];
	m_lich.parity        = m_lichBuffer[7];

//	if (m_lich.parity != (m_lichEvenParity % 2)) {
//		std::cerr << "DSDNXDN::processLICH: LICH parity error" << std::endl;
//	} else if (m_lich.rfChannelCode != 2) {
//		std::cerr << "DSDNXDN::processLICH: wrong RF channel type for RDCH: " << m_lich.rfChannelCode << std::endl;
//	}
}

} // namespace DSDcc

