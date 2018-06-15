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
		m_inSync(false),
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
    if (!m_inSync)
    {
        std::cerr << "DSDNXDN::init: entering sync state" << std::endl;
        m_inSync = true;
    }

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
        m_inSync = false;
    }
}

int DSDNXDN::unscrambleDibit(int dibit)
{
    //return dibit;
    return m_pn.getBit(m_symbolIndex) ? dibit ^ 2 : dibit; // apply PN scrambling. Inverting symbol is a XOR by 2 on the dibit.
}

void DSDNXDN::acquireLICH(int dibit)
{
    m_lichBuffer[m_symbolIndex] = dibit >> 1; // conversion is a divide by 2

    if (m_symbolIndex < 4) { // parity is on 4 MSB only
        m_lichEvenParity += m_lichBuffer[m_symbolIndex];
    }
}

void DSDNXDN::processFrame()
{
    int dibit = unscrambleDibit(m_dsdDecoder->m_dsdSymbol.getDibit());

	if (m_symbolIndex < 8) // LICH info
	{
	    acquireLICH(dibit);
		m_symbolIndex++;

		if (m_symbolIndex == 8) {
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
		m_inSync = false;
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
        m_inSync = false;
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
        m_symbolIndex = 0;
        m_lichEvenParity = 0;
        acquireLICH(unscrambleDibit(m_syncBuffer[9])); // re-introduce last symbol
        m_symbolIndex++;
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
        m_symbolIndex = 0;
        m_lichEvenParity = 0;
        acquireLICH(unscrambleDibit(m_syncBuffer[8])); // re-introduce symbol before last symbol
        m_symbolIndex++;
        acquireLICH(unscrambleDibit(m_syncBuffer[9])); // re-introduce last symbol
        m_symbolIndex++;
        m_state = NXDNFrame;
    }
    else
    {
        std::cerr << "DSDNXDN::processFSW: sync lost" << std::endl;
        m_dsdDecoder->resetFrameSync(); // end
        m_inSync = false;
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
        m_lichEvenParity += m_lich.parity; // you have to sum with parity bit and then test even-ness


//	if (m_lich.parity != (m_lichEvenParity % 2)) {
//		std::cerr << "DSDNXDN::processLICH: LICH parity error" << std::endl;
//	} else {
		std::cerr << "DSDNXDN::processLICH:"
		        << " rfChannelCode: " << m_lich.rfChannelCode
		        << " fnChannelCode: " << m_lich.fnChannelCode
		        << " optionCode: " << m_lich.optionCode
		        << " direction: " << m_lich.direction
		        << " parity: " << m_lich.parity
                        << " checked: " << (m_lichEvenParity % 2) << std::endl;
//	}
}

} // namespace DSDcc

