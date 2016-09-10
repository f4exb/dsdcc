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

#include "dsd_decoder.h"
#include "descramble.h"
#include "dstar.h"

namespace DSDcc
{

DSDDstar::DSDDstar(DSDDecoder *dsdDecoder) :
		DSDDstarOld(dsdDecoder),
		m_dsdDecoder(dsdDecoder)
{
}

DSDDstar::~DSDDstar()
{
}

void DSDDstar::init(bool header)
{
    //fprintf(stderr, "DSDDstar::init: symbol %d (%d)\n", m_dsdDecoder->m_state.symbolcnt, m_dsdDecoder->m_dsdSymbol.getSymbol());

    if (header)
    {
        framecount = 0;
    }
    else
    {
        framecount = 1; //just saw a sync frame; there should be 20 not 21 till the next
    }

    if ((m_dsdDecoder->m_opts.errorbars == 1) && (!header))
    {
        m_dsdDecoder->getLogger().log( "e:"); // print this only for voice/data frames
    }

    sync_missed = 0;
    bitbuffer = 0;
    m_symbolIndex = 0;
    m_symbolIndexHD = 0;
    m_dsdDecoder->m_voice1On = true;
}

void DSDDstar::processHD()
{
    if (m_symbolIndexHD == 660-1)
    {
        dstar_header_decode();
        init(); // init for DSTAR
        m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDSTAR; // go to DSTAR
    }
    else
    {
        m_symbolIndexHD++;
    }
}

void DSDDstar::dstar_header_decode()
{
    unsigned char radioheaderbuffer2[660];
    unsigned char radioheaderbuffer3[660];
    unsigned char radioheader[41];
    int octetcount, bitcount, loop;
    unsigned char bit2octet[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

    Descramble::scramble(m_dsdDecoder->m_dsdSymbol.getDibitBack(660), radioheaderbuffer2);
    Descramble::deinterleave(radioheaderbuffer2, radioheaderbuffer3);
    Descramble::FECdecoder(radioheaderbuffer3, radioheaderbuffer2);
    memset(radioheader, 0, 41);

    // note we receive 330 bits, but we only use 328 of them (41 octets)
    // bits 329 and 330 are unused
    octetcount = 0;
    bitcount = 0;

    for (loop = 0; loop < 328; loop++)
    {
        if (radioheaderbuffer2[loop])
        {
            radioheader[octetcount] |= bit2octet[bitcount];
        };

        bitcount++;

        // increase octetcounter and reset bitcounter every 8 bits
        if (bitcount >= 8)
        {
            octetcount++;
            bitcount = 0;
        }
    }

    // print header - TODO: store it in the state object for access from caller
    m_dsdDecoder->getLogger().log("\nDSTAR HEADER: ");
    m_rpt2 = std::string((const char *) &radioheader[3], 8);
    m_dsdDecoder->getLogger().log("RPT 2: %s ", m_rpt2.c_str());
    m_rpt1 = std::string((const char *) &radioheader[11], 8);
    m_dsdDecoder->getLogger().log("RPT 1: %s ", m_rpt1.c_str());
    m_yourSign = std::string((const char *) &radioheader[19], 8);
    m_dsdDecoder->getLogger().log("YOUR: %s ", m_yourSign.c_str());
    m_mySign = std::string((const char *) &radioheader[27], 8);
    m_mySign += '/';
    m_mySign += std::string((const char *) &radioheader[35], 4);
    m_dsdDecoder->getLogger().log("MY: %s\n", m_mySign.c_str());
}

} // namespace DSDcc
