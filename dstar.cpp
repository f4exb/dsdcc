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
#include "dsd_decoder.h"
#include "descramble.h"
#include "dstar.h"

namespace DSDcc
{

const unsigned char DSDDstar::m_terminationSequence[48] = {
        3, 1,  3, 1,  3, 1,  3, 1,  3, 1,  3, 1,  3, 1,  3, 1,
        3, 1,  3, 1,  3, 1,  3, 1,  3, 1,  3, 1,  3, 1,  3, 1,
        1, 1,  1, 3,  1, 1,  3, 3,  1, 3,  1, 3,  3, 3,  3, 1,
};

const int DSDDstar::dW[72] = {

        // 0-11
        0, 0,
        3, 2,
        1, 1,
        0, 0,
        1, 1,
        0, 0,

        // 12-23
        3, 2,
        1, 1,
        3, 2,
        1, 1,
        0, 0,
        3, 2,

        // 24-35
        0, 0,
        3, 2,
        1, 1,
        0, 0,
        1, 1,
        0, 0,

        // 36-47
        3, 2,
        1, 1,
        3, 2,
        1, 1,
        0, 0,
        3, 2,

        // 48-59
        0, 0,
        3, 2,
        1, 1,
        0, 0,
        1, 1,
        0, 0,

        // 60-71
        3, 2,
        1, 1,
        3, 3,
        2, 1,
        0, 0,
        3, 3,
};

const int DSDDstar::dX[72] = {

        // 0-11
        10, 22,
        11, 9,
        10, 22,
        11, 23,
        8, 20,
        9, 21,

        // 12-23
        10, 8,
        9, 21,
        8, 6,
        7, 19,
        8, 20,
        9, 7,

        // 24-35
        6, 18,
        7, 5,
        6, 18,
        7, 19,
        4, 16,
        5, 17,

        // 36-47
        6, 4,
        5, 17,
        4, 2,
        3, 15,
        4, 16,
        5, 3,

        // 48-59
        2, 14,
        3, 1,
        2, 14,
        3, 15,
        0, 12,
        1, 13,

        // 60-71
        2, 0,
        1, 13,
        0, 12,
        10, 11,
        0, 12,
        1, 13,
};


DSDDstar::DSDDstar(DSDDecoder *dsdDecoder) :
		m_dsdDecoder(dsdDecoder),
		m_voiceFrameCount(0),
		m_frameType(DStarVoiceFrame),
		m_viterbi(2, Viterbi::Poly23a, false)
{
    reset_header_strings();
}

DSDDstar::~DSDDstar()
{
}

void DSDDstar::init(bool header)
{
    //fprintf(stderr, "DSDDstar::init: symbol %d (%d)\n", m_dsdDecoder->m_state.symbolcnt, m_dsdDecoder->m_dsdSymbol.getSymbol());

    if (header)
    {
        m_dsdDecoder->m_voice1On = false;
    }
    else
    {
        m_voiceFrameCount = 0;
        m_frameType = DStarVoiceFrame;
        m_dsdDecoder->m_voice1On = true;
    }

    if ((m_dsdDecoder->m_opts.errorbars == 1) && (!header))
    {
        m_dsdDecoder->getLogger().log( "e:"); // print this only for voice/data frames
    }

    bitbuffer = 0;
    m_symbolIndex = 0;
    m_symbolIndexHD = 0;
}

void DSDDstar::initVoiceFrame()
{
    memset(m_dsdDecoder->ambe_fr, 0, 96);
    memset((void *) m_dsdDecoder->m_mbeDVFrame1, 0, 9); // initialize DVSI frame
    // voice frame
    w = dW;
    x = dX;
}
void DSDDstar::initDataFrame()
{
}

void DSDDstar::reset_header_strings()
{
    m_rpt1.clear();
    m_rpt2.clear();
    m_yourSign.clear();
    m_mySign.clear();
}

void DSDDstar::process()
{
    if (m_frameType == DStarVoiceFrame)
    {
        processVoice();
    }
    else if (m_frameType == DStarDataFrame)
    {
        processData();
    }
    else if (m_frameType == DStarSyncFrame)
    {
        processSync();
    }
}

void DSDDstar::processVoice()
{
    unsigned char bit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get bit from symbol and store it in cache

    bitbuffer <<= 1;

    if (bit == 1) {
        bitbuffer |= 0x01;
    }

    if (m_symbolIndex == 0) {
        initVoiceFrame();
    }

    m_dsdDecoder->ambe_fr[*w][*x] = (1 & bit);
    w++;
    x++;

    storeSymbolDV(m_symbolIndex, (1 & bit)); // store bits in order in DVSI frame

    if (m_symbolIndex == 72-1)
    {
//        std::cerr << "DSDDstar::processVoice: " << m_voiceFrameCount << std::endl;

        if (m_dsdDecoder->m_opts.errorbars == 1) {
            m_dsdDecoder->getLogger().log("\nMBE: ");
        }

        m_dsdDecoder->m_mbeDecoder1.processFrame(0, m_dsdDecoder->ambe_fr, 0);
        m_dsdDecoder->m_mbeDVReady1 = true; // Indicate that a DVSI frame is available

        m_symbolIndex = 0;

        if (m_voiceFrameCount < 20)
        {
            m_voiceFrameCount++;
            m_frameType = DStarDataFrame;
        }
        else
        {
            m_frameType = DStarSyncFrame;
        }
    }
    else
    {
        m_symbolIndex++;
    }
}

void DSDDstar::processData()
{
    int bit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

    bitbuffer <<= 1;

    if (bit == 1) {
        bitbuffer |= 0x01;
    }

    if (m_symbolIndex == 24-1) // last bit in data frame
    {
//        std::cerr << "DSDDstar::processData: " << m_voiceFrameCount << std::endl;

        slowdata[0] = (bitbuffer >> 16) & 0x000000FF;
        slowdata[1] = (bitbuffer >> 8) & 0x000000FF;
        slowdata[2] = (bitbuffer) & 0x000000FF;
        slowdata[3] = 0;

        if ((m_voiceFrameCount > 1) && ((bitbuffer & 0x00FFFFFF) != 0x000000))
        {
            slowdata[0] ^= 0x70;
            slowdata[1] ^= 0x4f;
            slowdata[2] ^= 0x93;
            //printf("unscrambled- %s",slowdata);
        }

        m_symbolIndex = 0;
        m_frameType = DStarVoiceFrame;
    }
    else
    {
        m_symbolIndex++;
    }
}

void DSDDstar::processSync()
{
    if (m_symbolIndex >= 72) // we're lost
    {
        m_dsdDecoder->m_voice1On = false;
        reset_header_strings();
        m_dsdDecoder->resetFrameSync(); // end
        return;
    }

    if (m_symbolIndex >= 12)
    {
        if (memcmp(m_dsdDecoder->m_dsdSymbol.getNonInvertedSyncDibitBack(24), DSDDecoder::m_syncDStar, 24) == 0) // sync
        {
//            std::cerr << "DSDDstar::processSync: SYNC" << std::endl;

            m_symbolIndex = 0;
            m_voiceFrameCount = 0;
            m_frameType = DStarVoiceFrame;
            return;
        }
    }

    if (m_symbolIndex >= 36)
    {
        if (memcmp(m_dsdDecoder->m_dsdSymbol.getNonInvertedSyncDibitBack(48), m_terminationSequence, 48) == 0) // termination
        {
//            std::cerr << "DSDDstar::processSync: TERMINATE" << std::endl;

            m_dsdDecoder->m_voice1On = false;
            reset_header_strings();
            m_dsdDecoder->resetFrameSync(); // end
            return;
        }
    }

    m_symbolIndex++;
}

void DSDDstar::processHD()
{
    if (m_symbolIndexHD == 660-1)
    {
        dstar_header_decode();
        init(); // init for DSTAR
        m_frameType = DStarVoiceFrame; // we start on a voice frame
        m_voiceFrameCount = 20;        // we start at one frame before sync
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
//    Descramble::FECdecoder(radioheaderbuffer3, radioheaderbuffer2);
    m_viterbi.decodeFromBits(radioheaderbuffer2, radioheaderbuffer3, 660, 0);
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

void DSDDstar::storeSymbolDV(int bitindex, unsigned char bit, bool lsbFirst)
{
    if (lsbFirst)
    {
        m_dsdDecoder->m_mbeDVFrame1[bitindex/8] |= bit << (bitindex%8); // store bits in order in DVSI frame LSB first
    }
    else
    {
        m_dsdDecoder->m_mbeDVFrame1[8 - (bitindex/8)] |= bit << (7 - (bitindex%8)); // store bits in order in DVSI frame MSB first
    }
}

} // namespace DSDcc
