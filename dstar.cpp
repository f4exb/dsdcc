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
#include <math.h>

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
		m_symbolIndex(0),
		m_symbolIndexHD(0),
        m_viterbi(2, Viterbi::Poly23a, false),
        m_crc(CRC::PolyDStar16, 16, 0xffff, 0xffff, 1, 0, 0),
		slowdataIx(0),
		w(0),
		x(0)
{
    reset_header_strings();
    m_slowData.init();
    memset(nullBytes, 0, 4);
    memset(slowdata, 0, 4);
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
    std::cerr << "DSDDstar::reset_header_strings" << std::endl;
    m_header.clear();
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
            m_frameType = DStarDataFrame;
            m_voiceFrameCount++;
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

    if (m_symbolIndex == 0)
    {
        memset(slowdata, 0, 4);
        memset(nullBytes, 0, 4);
        slowdataIx = 0;
    }
    else if (m_symbolIndex%8 == 0)
    {
        slowdataIx++;
    }

    slowdata[slowdataIx] += bit<<(m_symbolIndex%8); // this is LSB first!

    if (m_symbolIndex == 24-1) // last bit in data frame
    {
//        std::cerr << "DSDDstar::processData: " << m_voiceFrameCount << std::endl;

        if ((m_voiceFrameCount > 0) && (memcmp(slowdata, nullBytes, 4) != 0))
        {
            slowdata[0] ^= 0x70;
            slowdata[1] ^= 0x4f;
            slowdata[2] ^= 0x93;
//            std::cerr << "DSDDstar::processData:"
//                    << " " << std::hex << (int) (slowdata[0])
//                    << " " << std::hex << (int) (slowdata[1])
//                    << " " << std::hex << (int) (slowdata[2])
//                    << " (" << m_voiceFrameCount << ")" << std::endl;
            processSlowData(m_voiceFrameCount == 1);
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

void DSDDstar::processSlowData(bool firstFrame)
{
    // byte 0
    if (firstFrame || (m_slowData.counter == 0))
    {
        int dataType = (slowdata[0] >> 4) & 0xF;
        m_slowData.counter = slowdata[0] & 0xF;

        if (dataType > 6)
        {
            m_slowData.currentDataType = DStarSlowDataNone;
        }
        else if (dataType == 6) // filler
        {
            m_slowData.currentDataType = DStarSlowDataFiller;
            m_slowData.counter = 2;
        }
        else if (dataType == 4) // text
        {
            m_slowData.textFrameIndex = m_slowData.counter % 4;
            m_slowData.counter = 5;
            m_slowData.currentDataType = (DStarSlowDataType) dataType;
        }
        else
        {
            m_slowData.currentDataType = (DStarSlowDataType) dataType;
        }

        if (firstFrame)
        {
            // unconditionnaly reset counters for elements that are always contained in the same 20 frame sequence
            m_slowData.radioHeaderIndex = 0;

            // initializations based on data type
            switch (m_slowData.currentDataType)
            {
            case DStarSlowDataHeader:
                if (!m_slowData.gpsStart)
                {
                    processDPRS();
                }
                m_slowData.gpsStart = true;
                break;
            case DStarSlowDataText:
                if (!m_slowData.gpsStart)
                {
                    processDPRS();
                }
                m_slowData.gpsStart = true;
                break;
            case DStarSlowDataGPS:
                if (m_slowData.gpsStart)
                {
                    m_slowData.gpsIndex = 0;
                    m_slowData.gpsStart = false;
                }
                break;
            default:
                break;
            }
        }

//        std::cerr << "DSDDstar::processSlowData: " << dataType << ":" << m_slowData.counter << std::endl;
    }
    else
    {
        if (m_slowData.counter > 0)
        {
            processSlowDataByte(slowdata[0]);
            m_slowData.counter--;
        }
    }
    // byte 1
    if (m_slowData.counter > 0)
    {
        processSlowDataByte(slowdata[1]);
        m_slowData.counter--;
    }
    // byte 2
    if (m_slowData.counter > 0)
    {
        processSlowDataByte(slowdata[2]);
        m_slowData.counter--;
    }

    processSlowDataGroup();
}

void DSDDstar::processSlowDataByte(unsigned char byte)
{
    switch (m_slowData.currentDataType)
    {
    case DStarSlowDataHeader:
        if (m_slowData.radioHeaderIndex < 41)
        {
            m_slowData.radioHeader[m_slowData.radioHeaderIndex] = byte < 32 || byte > 127 ? 46 : byte;
            m_slowData.radioHeaderIndex++;
        }
        break;
    case DStarSlowDataText:
        m_slowData.text[5*m_slowData.textFrameIndex + 5 -m_slowData.counter] = byte < 32 || byte > 127 ? 46 : byte;
        break;
    case DStarSlowDataGPS:
        m_slowData.gpsNMEA[m_slowData.gpsIndex] = byte;
        m_slowData.gpsIndex++;
        break;
    default:
        break;
    }
}

void DSDDstar::processSlowDataGroup()
{
    switch (m_slowData.currentDataType)
    {
    case DStarSlowDataHeader:
        if (m_slowData.radioHeaderIndex == 41) // last byte
        {
            if (m_crcDStar.check_crc((unsigned char *) m_slowData.radioHeader, 41))
        	{
//                std::cerr << "DSDDstar::processSlowDataGroup: DStarSlowDataHeader OK" << std::endl;
                m_header.setRpt2((const char *) &m_slowData.radioHeader[3], false);
                m_header.setRpt1((const char *) &m_slowData.radioHeader[11], false);
                m_header.setYourSign((const char *) &m_slowData.radioHeader[19], false);
                m_header.setMySign((const char *) &m_slowData.radioHeader[27], (const char *) &m_slowData.radioHeader[35], false);
        	}
//            else
//            {
//                std::cerr << "DSDDstar::processSlowDataGroup: DStarSlowDataHeader KO" << std::endl;
//            }

            m_slowData.radioHeaderIndex = 0; // this is normally done on the first frame though
        }
        break;
    case DStarSlowDataText:
        m_slowData.text[20] = '\0';
//        std::cerr << "DSDDstar::processSlowDataGroup: DStarSlowDataText: " << m_slowData.text << std::endl;
        break;
    default:
        break;
    }
}

void DSDDstar::processSync()
{
    if (m_symbolIndex >= 72) // we're lost
    {
        m_dsdDecoder->m_voice1On = false;
        reset_header_strings();
        m_slowData.init();
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
            m_slowData.init();
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
        reset_header_strings();
        m_slowData.init();
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

    m_dsdDecoder->getLogger().log("\nDSTAR HEADER: ");

    m_header.setRpt2((const char *) &radioheader[3], true);
    m_dsdDecoder->getLogger().log("RPT 2: %s ", m_header.m_rpt2.c_str());

    m_header.setRpt1((const char *) &radioheader[11], true);
    m_dsdDecoder->getLogger().log("RPT 1: %s ", m_header.m_rpt1.c_str());

    m_header.setYourSign((const char *) &radioheader[19], true);
    m_dsdDecoder->getLogger().log("YOUR: %s ", m_header.m_yourSign.c_str());

    m_header.setMySign((const char *) &radioheader[27], (const char *) &radioheader[35], true);
    m_dsdDecoder->getLogger().log("MY: %s\n", m_header.m_mySign.c_str());
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

void DSDDstar::processDPRS()
{
    m_slowData.gpsNMEA[m_slowData.gpsIndex] = '\0';

    if (memcmp(m_slowData.gpsNMEA, "$$CRC", 5) == 0)
    {
//        std::cerr << "DSDDstar::processDPRS: " << m_slowData.gpsNMEA << std::endl;
        bool crcOK = m_crcDStar.check_crc(
                (unsigned char *) &m_slowData.gpsNMEA[10],
                (int) strlen(m_slowData.gpsNMEA)-10,
                m_dprs.getCRC(&m_slowData.gpsNMEA[5]));

        if (crcOK)
        {
            if (m_dprs.matchDSTAR(m_slowData.gpsNMEA))
            {
                m_dprs.m_locPoint.getLocator().toCSting(m_slowData.locator);
                m_slowData.bearing = m_dsdDecoder->m_myPoint.bearingTo(m_dprs.m_locPoint);
                m_slowData.distance = m_dsdDecoder->m_myPoint.distanceTo(m_dprs.m_locPoint);
//                std::cerr << "DSDDstar::processDPRS: " << m_dprs.lat << ":" << m_dprs.lon << ":" <<  m_dprs.m_locator.toString() << std::endl;
            }
        }
    }
}

unsigned int DSDDstar::DPRS::getCRC(const char *d)
{
    char crcStr[5];
    memcpy(crcStr, d, 4);
    crcStr[4] = '\0';

    return (unsigned int) strtol(crcStr, 0, 16);
}

bool DSDDstar::DPRS::matchDSTAR(const char *d)
{
    const char *pch;
    char latStr[7+1];
    char lonStr[8+1];
    char latH, lonH;
    double x, min, deg;

    pch = strstr (d, "DSTAR*:/");

    if (pch)
    {
        pch += 15;

        memcpy(latStr, pch, 7);
        latStr[7] = '\0';
        latH = pch[7];
        x = atof(latStr);
        x /= 100.0f;
        min = modf(x, &deg);
        m_lat = (deg + ((min*100.0f)/60.0f))*(latH == 'N' ? 1 : -1);

        pch += 9;

        memcpy(lonStr, pch, 8);
        lonStr[8] = '\0';
        lonH = pch[8];
        x = atof(lonStr);
        x /= 100.0f;
        min = modf(x, &deg);
        m_lon = (deg + ((min*100.0f)/60.0f))*(lonH == 'E' ? 1 : -1);

        m_locPoint.setLatLon(m_lat, m_lon);

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace DSDcc
