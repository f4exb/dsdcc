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
#include <string.h>
#include "dmr.h"
#include "dsd_decoder.h"

namespace DSDcc
{

const int DSDDMR::m_cachInterleave[24] = {0, 4, 8, 12, 14, 18, 22, 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 15, 16, 17, 19, 20, 21, 23};
const char *DSDDMR::m_slotTypeText[13] = {
        "PIH",
        "VLC",
        "TLC",
        "CSB",
        "MBH",
        "MBC",
        "DAH",
        "D12",
        "D34",
        "IDL",
        "D01",
        "RES",
        "UNK"
};

const unsigned char DSDDMR::Hamming_7_4::m_H[7*3] = {
        1, 1, 1, 0,   1, 0, 0,
        0, 1, 1, 1,   0, 1, 0,
        1, 1, 0, 1,   0, 0, 1
//      0  1  2  3 <- correctable bit positions
};

void DSDDMR::Hamming_7_4::init()
{
    // correctable bit positions given syndrome bits as index (see above)
    memset(m_corr, 0xFF, 8); // initialize with all invalid positions
    m_corr[0b101] = 0;
    m_corr[0b111] = 1;
    m_corr[0b110] = 2;
    m_corr[0b011] = 3;
}

// ========================================================================================

const unsigned char DSDDMR::Golay_20_8::m_H[20*12] = {
        0, 1, 0, 0, 1, 1, 1, 1,    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 0, 1, 0, 0, 0,    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 1, 1, 0, 1, 0, 0,    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 0, 1, 1, 0, 1, 0,    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 0, 1, 1, 0, 1,    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 1, 1, 1, 0, 0, 1,    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 1, 1,    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
        1, 1, 0, 0, 0, 1, 1, 0,    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        1, 1, 1, 0, 0, 0, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
        1, 0, 0, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
        0, 1, 1, 1, 0, 1, 0, 1,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
};

// ========================================================================================

DSDDMR::DSDDMR(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbolIndex(0),
        m_burstType(DSDDMRBurstNone),
        m_slot(DSDDMRSlotUndefined),
        m_lcss(0),
        m_colorCode(0),
        m_dataType(DSDDMRDataUnknown)
{
    m_slotText = m_dsdDecoder->m_state.slot0light;
    m_slotTextIndex = 0;
}

DSDDMR::~DSDDMR()
{
}

void DSDDMR::initData(DSDDMRBurstType burstType)
{
    m_burstType = burstType;
    m_slotText = m_dsdDecoder->m_state.slot0light;
    m_slotTextIndex = 0;

    processDataFirstHalf();

    m_symbolIndex = 13; // start counting right of center as in ETSI TS 102 361-1 p.58
}

void DSDDMR::processData()
{
    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

    if (m_symbolIndex < 18)
    {
        m_slotTypePDU_dibits[5 + m_symbolIndex - 13] = dibit;

        if (m_symbolIndex == 17)
        {
            processSlotTypePDU();
        }
    }
    else if (m_symbolIndex == 66) // last dibit
    {
        m_dsdDecoder->resetFrameSync(); // end
    }

    m_symbolIndex++;
}

void DSDDMR::processVoice()
{

}

void DSDDMR::processDataFirstHalf()
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(90+1);

    if (m_burstType == DSDDMRBaseStation) // CACH is for base station only
    {
        processCACH(dibit_p);
    }

    dibit_p += 12; // move after either CACH or garbage

    dibit_p += 49; // move after first half info block

    memcpy((void *) m_slotTypePDU_dibits, (const void *) dibit_p, 5);
    processSlotTypePDU();

    dibit_p += 5;  // move after first half slot type PDU

    dibit_p += 24; // move after SYNC or embedded signaling
}

void DSDDMR::processVoiceFirstHalf()
{
}

void DSDDMR::processCACH(unsigned char *dibit_p)
{
    unsigned char cachBits[24];

    for (int i = 0; i < 12; i++)
    {
        cachBits[m_cachInterleave[2*i]]   = (dibit_p[i] >> 1) & 1;
        cachBits[m_cachInterleave[2*i+1]] = dibit_p[i] & 1;
    }

    // Hamming (7,4) decode and store results if successful
    if (m_hamming_7_4.decode(cachBits))
    {
        unsigned int slotIndex = cachBits[1] & 1;
        m_dsdDecoder->m_state.currentslot = slotIndex; // FIXME: remove this when done with new voice processing

        if (slotIndex)
        {
            m_slotText = m_dsdDecoder->m_state.slot1light;
        }
        else
        {
            m_slotText = m_dsdDecoder->m_state.slot0light;
        }

        m_slotText[0] = ((cachBits[0] & 1) ? '*' : '.');
        m_slot = (DSDDMRSlot) (cachBits[1] + 1);
        m_lcss = 2*cachBits[2] + cachBits[3];

//        std::cerr << "DSDDMR::processCACH: OK: Slot: " << (int) cachBits[1] << " LCSS: " << (int) m_lcss << std::endl;
    }
    else
    {
        m_slot = DSDDMRSlotUndefined;
        m_slotText = m_dsdDecoder->m_state.slot0light;
        m_dsdDecoder->m_state.slot0light[0] = '/';
//        std::cerr << "DSDDMR::processCACH: KO" << std::endl;
    }
}

void DSDDMR::processSlotTypePDU()
{
    unsigned char slotTypeBits[20];

    for (int i = 0; i < 10; i++)
    {
        slotTypeBits[2*i]     = (m_slotTypePDU_dibits[i] >> 1) & 1;
        slotTypeBits[2*i + 1] = m_slotTypePDU_dibits[i] & 1;
    }

    if (m_golay_20_8.decode(slotTypeBits))
    {
        m_colorCode = (slotTypeBits[0] << 3) + (slotTypeBits[1] << 2) + (slotTypeBits[2] << 1) + slotTypeBits[3];
        sprintf(&m_slotText[1], "%02d ", m_colorCode);

        unsigned int dataType = (slotTypeBits[4] << 3) + (slotTypeBits[5] << 2) + (slotTypeBits[6] << 1) + slotTypeBits[7];

        if (dataType > 10)
        {
            m_dataType = DSDDMRDataReserved;
            memcpy(&m_slotText[4], "RES", 3);
        }
        else
        {
            m_dataType = (DSDDMRDataTYpe) dataType;
            memcpy(&m_slotText[4], m_slotTypeText[dataType], 3);
        }

//        std::cerr << "DSDDMR::processSlotTypePDU OK: CC: " << (int) m_colorCode << " DT: " << dataType << std::endl;
    }
    else
    {
        memcpy(&m_slotText[1], "-- UNK", 6);
//        std::cerr << "DSDDMR::processSlotTypePDU KO" << std::endl;
    }
}

// ========================================================================================

DSDDMR::Hamming_7_4::Hamming_7_4()
{
    init();
}

DSDDMR::Hamming_7_4::~Hamming_7_4()
{
}

bool DSDDMR::Hamming_7_4::decode(unsigned char *rxBits) // corrects in place
{
    unsigned int syndromeI = 0; // syndrome index

    for (int is = 0; is < 3; is++)
    {
        syndromeI += (((rxBits[0] * m_H[7*is + 0])
                    + (rxBits[1] * m_H[7*is + 1])
                    + (rxBits[2] * m_H[7*is + 2])
                    + (rxBits[3] * m_H[7*is + 3])
                    + (rxBits[4] * m_H[7*is + 4])
                    + (rxBits[5] * m_H[7*is + 5])
                    + (rxBits[6] * m_H[7*is + 6])) % 2) << is;
    }

    if (syndromeI > 0)
    {
        if (m_corr[syndromeI] == 0xFF)
        {
            return false;
        }
        else
        {
            rxBits[m_corr[syndromeI]] ^= 1; // flip bit
        }
    }

    return true;
}

// ========================================================================================

DSDDMR::Golay_20_8::Golay_20_8()
{
    init();
}

DSDDMR::Golay_20_8::~Golay_20_8()
{
}

void DSDDMR::Golay_20_8::init()
{
    memset (m_corr, 0xFF, 3*4096);

    for (int i1 = 0; i1 < 8; i1++)
    {
        for (int i2 = i1+1; i2 < 8; i2++)
        {
            for (int i3 = i2+1; i3 < 8; i3++)
            {
                // 3 bit patterns
                int syndromeI = 0;

                for (int ir = 0; ir < 12; ir++)
                {
                    syndromeI += ((m_H[20*ir + i1] +  m_H[20*ir + i2] +  m_H[20*ir + i3]) % 2) << (11-ir);
                }

                m_corr[syndromeI][0] = i1;
                m_corr[syndromeI][1] = i2;
                m_corr[syndromeI][2] = i3;
            }

            // 2 bit patterns
            int syndromeI = 0;

            for (int ir = 0; ir < 12; ir++)
            {
                syndromeI += ((m_H[20*ir + i1] +  m_H[20*ir + i2]) % 2) << (11-ir);
            }

            m_corr[syndromeI][0] = i1;
            m_corr[syndromeI][1] = i2;
        }

        // single bit patterns
        int syndromeI = 0;

        for (int ir = 0; ir < 12; ir++)
        {
            syndromeI += m_H[20*ir + i1] << (11-ir);
        }

        m_corr[syndromeI][0] = i1;
    }
}

bool DSDDMR::Golay_20_8::decode(unsigned char *rxBits)
{
    unsigned int syndromeI = 0; // syndrome index

    for (int is = 0; is < 12; is++)
    {
        syndromeI += (((rxBits[0] * m_H[20*is + 0])
                    + (rxBits[1] * m_H[20*is + 1])
                    + (rxBits[2] * m_H[20*is + 2])
                    + (rxBits[3] * m_H[20*is + 3])
                    + (rxBits[4] * m_H[20*is + 4])
                    + (rxBits[5] * m_H[20*is + 5])
                    + (rxBits[6] * m_H[20*is + 6])
                    + (rxBits[7] * m_H[20*is + 7])
                    + (rxBits[8] * m_H[20*is + 8])
                    + (rxBits[9] * m_H[20*is + 9])
                    + (rxBits[10] * m_H[20*is + 10])
                    + (rxBits[11] * m_H[20*is + 11])
                    + (rxBits[12] * m_H[20*is + 12])
                    + (rxBits[13] * m_H[20*is + 13])
                    + (rxBits[14] * m_H[20*is + 14])
                    + (rxBits[15] * m_H[20*is + 15])
                    + (rxBits[16] * m_H[20*is + 16])
                    + (rxBits[17] * m_H[20*is + 17])
                    + (rxBits[18] * m_H[20*is + 18])
                    + (rxBits[19] * m_H[20*is + 19])) % 2) << is;
    }

    if (syndromeI > 0)
    {
        int i = 0;

        for (; i < 3; i++)
        {
            if (m_corr[syndromeI][i] == 0xFF)
            {
                break;
            }
            else
            {
                rxBits[m_corr[syndromeI][i]] ^= 1; // flip bit
            }
        }

        if (i == 0)
        {
            return false;
        }
    }

    return true;
}

} // namespace DSDcc

