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

const int DSDDMR::m_cachInterleave[24]   = {0, 7, 8, 9, 1, 10, 11, 12, 2, 13, 14, 15, 3, 16, 4, 17, 18, 19, 5, 20, 21, 22, 6, 23};
const int DSDDMR::m_embSigInterleave[128] = {
        0,  16,  32,  48,  64,  80,  96, 112,
        1,  17,  33,  49,  65,  81,  97, 113,
        2,  18,  34,  50,  66,  82,  98, 114,
        3,  19,  35,  51,  67,  83,  99, 115,
        4,  20,  36,  52,  68,  84, 100, 116,
        5,  21,  37,  53,  69,  85, 101, 117,
        6,  22,  38,  54,  70,  86, 102, 118,
        7,  23,  39,  55,  71,  87, 103, 119,
        8,  24,  40,  56,  72,  88, 104, 120,
        9,  25,  41,  57,  73,  89, 105, 121,
       10,  26,  42,  58,  74,  90, 106, 122,
       11,  27,  43,  59,  75,  91, 107, 123,
       12,  28,  44,  60,  76,  92, 108, 124,
       13,  29,  45,  61,  77,  93, 109, 125,
       14,  30,  46,  62,  78,  94, 110, 126,
       15,  31,  47,  63,  79,  95, 111, 127,
};
//ETSI TS 102 361-1 9.3.6. Data Type
const char *DSDDMR::m_slotTypeText[DMR_TYPES_COUNT] = {
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
        "USB"
};

/*
 * DMR AMBE interleave schedule
 */
// bit 1
const int DSDDMR::rW[36] = {
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 2,
  0, 2, 0, 2, 0, 2,
  0, 2, 0, 2, 0, 2
};

const int DSDDMR::rX[36] = {
  23, 10, 22, 9, 21, 8,
  20, 7, 19, 6, 18, 5,
  17, 4, 16, 3, 15, 2,
  14, 1, 13, 0, 12, 10,
  11, 9, 10, 8, 9, 7,
  8, 6, 7, 5, 6, 4
};

// bit 0
const int DSDDMR::rY[36] = {
  0, 2, 0, 2, 0, 2,
  0, 2, 0, 3, 0, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3
};

const int DSDDMR::rZ[36] = {
  5, 3, 4, 2, 3, 1,
  2, 0, 1, 13, 0, 12,
  22, 11, 21, 10, 20, 9,
  19, 8, 18, 7, 17, 6,
  16, 5, 15, 4, 14, 3,
  13, 2, 12, 1, 11, 0
};

// MotoTRBO algo only. Hytera don`t use hardcoded keys
const unsigned short DSDDMR::BasicPrivacyKeys[DMR_BP_KEYS_COUNT] = {
  0x1F00, 0xE300, 0xFC00, 0x2503, 0x3A03, 0xC603, 0xD903, 0x4A05, 0x5505, 0xA905,
  0xB605, 0x6F06, 0x7006, 0x8C06, 0x9306, 0x2618, 0x3918, 0xC518, 0xDA18, 0x031B,
  0x1C1B, 0xE01B, 0xFF1B, 0x6C1D, 0x731D, 0x8F1D, 0x901D, 0x491E, 0x561E, 0xAA1E,
  0xB51E, 0x4B28, 0x5428, 0xA828, 0xB728, 0x6E2B, 0x712B, 0x8D2B, 0x922B, 0x012D,
  0x1E2D, 0xE22D, 0xFD2D, 0x242E, 0x3B2E, 0xC72E, 0xD82E, 0x6D30, 0x7230, 0x8E30,
  0x9130, 0x4833, 0x5733, 0xAB33, 0xB433, 0x2735, 0x3835, 0xC435, 0xDB35, 0x0236,
  0x1D36, 0xE136, 0xFE36, 0x2B49, 0x3449, 0xC849, 0xD749, 0x0E4A, 0x114A, 0xED4A,
  0xF24A, 0x614C, 0xAE4C, 0x824C, 0x9D4C, 0x444F, 0x5B4F, 0xA74F, 0xB84F, 0x0D51,
  0x1251, 0xEE51, 0xF151, 0x2852, 0x3752, 0xCB52, 0xD452, 0x4754, 0x5854, 0xA454,
  0xBB54, 0x6257, 0x7D57, 0x8157, 0x9E57, 0x6061, 0x7F61, 0x8361, 0x9C61, 0x4562,
  0x5A62, 0xA662, 0xB962, 0x2A64, 0x3564, 0xC964, 0xD664, 0x0F67, 0x1067, 0xEC67,
  0xF367, 0x4679, 0x5979, 0xA579, 0xBA79, 0x637A, 0x7C7A, 0x807A, 0x9F7A, 0x0C7C,
  0x137C, 0xEF7C, 0xF07C, 0x297F, 0x367F, 0xCA7F, 0xD57F, 0x4D89, 0x5289, 0xAE89,
  0xB189, 0x688A, 0x778A, 0x8B8A, 0x948A, 0x078C, 0x188C, 0xE48C, 0xFB8C, 0x228F,
  0x3D8F, 0xC18F, 0xDE8F, 0x6B91, 0x7491, 0x8891, 0x9791, 0x4E92, 0x5192, 0xAD92,
  0xB292, 0x2194, 0x3E94, 0xC294, 0xDD94, 0x0497, 0x1B97, 0xE797, 0xF897, 0x06A1,
  0x19A1, 0xE5A1, 0xFAA1, 0x23A2, 0x3CA2, 0xC0A2, 0xDFA2, 0x4CA4, 0x53A4, 0xAFA4,
  0xB0A4, 0x69A7, 0x76A7, 0x8AA7, 0x95A7, 0x20B9, 0x3FB9, 0xC3B9, 0xDCB9, 0x05BA,
  0x1ABA, 0xE6BA, 0xF9BA, 0x6ABC, 0x75BC, 0x89BC, 0x96BC, 0x4FBF, 0x50BF, 0xACBF,
  0xB3BF, 0x66C0, 0x79C0, 0x85C0, 0x9AC0, 0x43C3, 0x5CC3, 0xA0C3, 0xBFC3, 0x2CC5,
  0x33C5, 0xCFC5, 0x0DC0, 0x09C6, 0x16C6, 0xEAC6, 0xF5C6, 0x84D0, 0x85DF, 0x8AD3,
  0x8BDC, 0xB6D5, 0xB7DA, 0xB8D6, 0xB9D9, 0x0DDA, 0xD1D5, 0xDED9, 0xDFD6, 0xE2DF,
  0xE3D0, 0xECDC, 0xEDD3, 0x2DE8, 0x32E8, 0xCEE8, 0xD1E8, 0x08EB, 0x17EB, 0xEBEB,
  0xF4EB, 0x67ED, 0x78ED, 0x84ED, 0x9BED, 0x42EE, 0x5DEE, 0xA1EE, 0xBEEE, 0x0BF0,
  0x14F0, 0xE8F0, 0xF7F0, 0x2EF3, 0x31F3, 0xCDF3, 0xD2F3, 0x41F5, 0x5EF5, 0xA2F5,
  0xBDF5, 0x64F6, 0x7BF6, 0x87F6, 0x98F6
};
// ========================================================================================

DSDDMR::DSDDMR(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbolIndex(0),
        m_cachSymbolIndex(0),
        m_burstType(DSDDMRBurstNone),
        m_slot(DSDDMRSlotUndefined),
        m_continuation(false),
        m_cachOK(false),
        m_lcss(SingleLC_FirstCSBK),
        m_colorCode(0),
        m_dataType(DSDDMRDataUnknown),
        m_voice1EmbSig_dibitsIndex(0),
        m_voice1EmbSig_OK(false),
        m_voice2EmbSig_dibitsIndex(0),
        m_voice2EmbSig_OK(false),
        m_voice1FrameCount(DMR_VOX_SUPERFRAME_LEN),
        m_voice2FrameCount(DMR_VOX_SUPERFRAME_LEN)
{
    m_slotText = m_dsdDecoder->m_state.slot0light;
    w = 0;
    x = 0;
    y = 0;
    z = 0;

    memset(m_slotTypePDU_dibits, 0, 10);
    memset(m_cachBits, 0, 24);
    memset(m_emb_dibits, 0, 8);
    memset(m_voiceEmbSig_dibits, 0, 16);
    memset(m_voice1EmbSigRawBits, 0, 16*8);
    memset(m_voice2EmbSigRawBits, 0, 16*8);
    memset(m_syncDibits, 0, 24);
    memset(m_mbeDVFrame, 0, 9);
}

DSDDMR::~DSDDMR()
{
}

void DSDDMR::initData()
{
//    std::cerr << "DSDDMR::initData" << std::endl;
    m_burstType = DSDDMRBaseStation;
    processDataFirstHalf(90+1);
}

void DSDDMR::initDataMS()
{
//    std::cerr << "DSDDMR::initDataMS" << std::endl;
    m_burstType = DSDDMRMobileStation;
    processDataFirstHalfMS();
}

void DSDDMR::initVoice()
{
//    std::cerr << "DSDDMR::initVoice" << std::endl;
    m_burstType = DSDDMRBaseStation;
    processVoiceFirstHalf(90+1);
}

void DSDDMR::initVoiceMS()
{
//    std::cerr << "DSDDMR::initVoiceMS" << std::endl;
    m_burstType = DSDDMRMobileStation;
    processVoiceFirstHalfMS();
}

void DSDDMR::processData()
{
    if ((!m_cachOK) && (m_burstType == DSDDMRBaseStation))
    {
        m_slotText = m_dsdDecoder->m_state.slot0light;
        memcpy(m_dsdDecoder->m_state.slot0light, "/-- UNK", 7);
        m_dsdDecoder->resetFrameSync();
        return; // abort
    }

    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

    processDataDibit(dibit);

    if (m_symbolIndex == IN_DIBITS(DMR_TS_LEN) - 1) // last dibit
    {
        if (m_slot == DSDDMRSlot1)
        {
            if (m_voice1FrameCount < DMR_VOX_SUPERFRAME_LEN) // continuation expected on slot + 2
            {
                std::cerr << "DSDDMR::processData: error: remaining voice in slot1" << std::endl;

                if (m_voice2FrameCount < DMR_VOX_SUPERFRAME_LEN)
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 2
                    m_continuation = true;
                }
                else
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRsyncOrSkip; // sync lookup or skip in slot 2
                    m_continuation = false; // TODO: true or false?
                }
            }
            else
            {
                if (m_voice2FrameCount < DMR_VOX_SUPERFRAME_LEN)
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 2
                    m_continuation = true;
                }
                else
                {
                    m_dsdDecoder->resetFrameSync(); // back to sync
                    m_continuation = false;
                }
            }
        }
        else if (m_slot == DSDDMRSlot2)
        {
            if (m_voice2FrameCount < DMR_VOX_SUPERFRAME_LEN) // continuation expected on slot + 2
            {
                std::cerr << "DSDDMR::processData: error: remaining voice in slot2" << std::endl;

                if (m_voice1FrameCount < DMR_VOX_SUPERFRAME_LEN)
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 1
                    m_continuation = true;
                }
                else
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRsyncOrSkip; // sync lookup or skip in slot 1
                    m_continuation = false; // TODO: true or false?
                }
            }
            else
            {
                if (m_voice1FrameCount < DMR_VOX_SUPERFRAME_LEN)
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 1
                    m_continuation = true;
                }
                else
                {
                    m_dsdDecoder->resetFrameSync(); // back to sync
                    m_continuation = false;
                }
            }
        }

        m_symbolIndex = 0;
    }
    else
    {
        m_symbolIndex++;
    }

    m_cachSymbolIndex++; // last dibit counts
}

void DSDDMR::processDataMS()
{
    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

    processDataDibit(dibit);

    if (m_symbolIndex == IN_DIBITS(DMR_TS_LEN) - 1) // last dibit
    {
        m_dsdDecoder->resetFrameSync(); // back to sync
        m_symbolIndex = 0;
    }
    else
    {
        m_symbolIndex++;
    }
}

void DSDDMR::processVoice()
{
    if ((!m_cachOK) && (m_burstType == DSDDMRBaseStation))
    {
        m_slotText = m_dsdDecoder->m_state.slot0light;
        memcpy(m_dsdDecoder->m_state.slot0light, "/-- UNK", 7);
        m_voice1FrameCount = DMR_VOX_SUPERFRAME_LEN;
        m_voice2FrameCount = DMR_VOX_SUPERFRAME_LEN;
        m_dsdDecoder->resetFrameSync();
        return; // abort
    }

    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

    processVoiceDibit(dibit);

    if (m_symbolIndex == IN_DIBITS(DMR_TS_LEN) - 1) // last dibit
    {
        if (m_slot == DSDDMRSlot1)
        {
            m_voice1FrameCount++;

            if (m_voice1FrameCount < DMR_VOX_SUPERFRAME_LEN) // continuation expected on slot + 2
            {
                if (m_voice2FrameCount < DMR_VOX_SUPERFRAME_LEN)
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 2
                    m_continuation = true;
                }
                else
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRsyncOrSkip; // sync lookup or skip in slot 2
                    m_continuation = false; // TODO: true or false ?
                }
            }
            else // no super frame on going on this slot
            {
                m_dsdDecoder->m_voice1On = false;

                if (m_voice2FrameCount < DMR_VOX_SUPERFRAME_LEN)
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 2
                    m_continuation = true;
                }
                else
                {
                    m_dsdDecoder->resetFrameSync(); // back to sync
                    m_continuation = false;
                }
            }
        }
        else if (m_slot == DSDDMRSlot2)
        {
            m_voice2FrameCount++;

            if (m_voice2FrameCount < DMR_VOX_SUPERFRAME_LEN) // continuation expected on slot + 2
            {
                if (m_voice1FrameCount < DMR_VOX_SUPERFRAME_LEN)
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 1
                    m_continuation = true;
                }
                else
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRsyncOrSkip; // sync lookup or skip in slot 1
                    m_continuation = false; // TODO: true or false ?
                }
            }
            else
            {
                m_dsdDecoder->m_voice2On = false;

                if (m_voice1FrameCount < DMR_VOX_SUPERFRAME_LEN)
                {
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 1
                    m_continuation = true;
                }
                else
                {
                    m_dsdDecoder->resetFrameSync(); // back to sync
                    m_continuation = false;
                }
            }
        }

        m_symbolIndex = 0;
    }
    else
    {
        m_symbolIndex++;
    }

    m_cachSymbolIndex++; // last dibit counts
}

void DSDDMR::processSyncOrSkip()
{
    const int sync_db_size = IN_DIBITS(DMR_SYNC_LEN);
    if (m_symbolIndex > sync_db_size) // accumulate enough symbols to look for a sync
    {
        if (memcmp(m_dsdDecoder->m_dsdSymbol.getSyncDibitBack(sync_db_size),
            DSDDecoder::m_syncDMRDataBS, sync_db_size) == 0)
        {
//		    std::cerr << "DSDDMR::processSyncOrSkip: data sync" << std::endl;
            processDataFirstHalf(90);
            m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRdata;
            return;
        }
        else if (memcmp(m_dsdDecoder->m_dsdSymbol.getSyncDibitBack(sync_db_size),
            DSDDecoder::m_syncDMRVoiceBS, sync_db_size) == 0)
        {
//		    std::cerr << "DSDDMR::processSyncOrSkip: voice sync" << std::endl;
            processVoiceFirstHalf(90);
            m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice;
            return;
        }
    }

    if (m_symbolIndex == IN_DIBITS(DMR_TS_LEN) - 1) // last dibit
    {
        // return to voice super frame
        m_slot = (DSDDMRSlot) (((int) m_slot + 1) % 2); // to keep the slot in the next slot period fake a slot reversal
        m_continuation = true;
        m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice;
        m_symbolIndex = 0;
    }
    else
    {
        m_symbolIndex++;
    }

    m_cachSymbolIndex++; // last dibit counts
}

void DSDDMR::processVoiceMS()
{
    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

    processVoiceDibit(dibit);

    if (m_symbolIndex == IN_DIBITS(DMR_TS_LEN) - 1) // last dibit
    {
        m_voice1FrameCount++;
//        std::cerr << "DSDDMR::processVoiceMS: " << m_symbolIndex << " : " << m_voice1FrameCount << std::endl;

        if (m_voice1FrameCount < DMR_VOX_SUPERFRAME_LEN) // continuation expected on slot + 2
        {
            m_dsdDecoder->m_dsdSymbol.setNoSignal(true);
            m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRSkipMS; // skip next slot
        }
        else // no super frame on going on this slot
        {
            m_dsdDecoder->m_voice1On = false;
            m_dsdDecoder->resetFrameSync(); // back to sync
        }

        m_symbolIndex = 0;
    }
    else
    {
        m_symbolIndex++;
    }
}

void DSDDMR::processSkipMS()
{

    if (m_symbolIndex == IN_DIBITS(DMR_TS_LEN) - 1) // last dibit
    {
//        std::cerr << "DSDDMR::processSkipMS: " << m_symbolIndex << std::endl;
        // return to voice super frame
        m_dsdDecoder->m_dsdSymbol.setNoSignal(false);
        m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoiceMS;
        m_symbolIndex = 0;
    }
    else
    {
        m_symbolIndex++;
    }
}

void DSDDMR::processDataFirstHalf(unsigned int shiftBack)
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(shiftBack);

//    std::cerr << "DSDDMR::processDataFirstHalf" << std::endl;

    for (m_symbolIndex = 0; m_symbolIndex < 90; m_symbolIndex++, m_cachSymbolIndex++)
    {
        processDataDibit(dibit_p[m_symbolIndex]);
    }
}

void DSDDMR::processDataFirstHalfMS()
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(78+1);

//    std::cerr << "DSDDMR::processDataFirstHalfMS" << std::endl;

    for (m_symbolIndex = 12; m_symbolIndex < 90; m_symbolIndex++, m_cachSymbolIndex++)
    {
        processDataDibit(dibit_p[m_symbolIndex]);
    }
}

void DSDDMR::processVoiceFirstHalf(unsigned int shiftBack)
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(shiftBack);

//    std::cerr << "DSDDMR::processVoiceFirstHalf" << std::endl;

    for (m_symbolIndex = 0; m_symbolIndex < 90; m_symbolIndex++, m_cachSymbolIndex++)
    {
        processVoiceDibit(dibit_p[m_symbolIndex]);
    }

    if (m_slot == DSDDMRSlot1)
    {
        m_voice1FrameCount = 0;
        m_dsdDecoder->m_voice1On = true;
        m_voice1EmbSig_dibitsIndex = 0;
        m_voice1EmbSig_OK = true;
    }
    else if (m_slot == DSDDMRSlot2)
    {
        m_voice2FrameCount = 0;
        m_dsdDecoder->m_voice2On = true;
        m_voice2EmbSig_dibitsIndex = 0;
        m_voice2EmbSig_OK = true;
    }
    else // invalid
    {
        m_voice1FrameCount = DMR_VOX_SUPERFRAME_LEN;
        m_voice2FrameCount = DMR_VOX_SUPERFRAME_LEN;
        m_dsdDecoder->m_voice1On = false;
        m_dsdDecoder->m_voice2On = false;
        m_voice1EmbSig_OK = false;
        m_voice2EmbSig_OK = false;
    }

}

void DSDDMR::processVoiceFirstHalfMS()
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(78+1); // no CACH with MS

    for (m_symbolIndex = 12; m_symbolIndex < 90; m_symbolIndex++, m_cachSymbolIndex++)
    {
        processVoiceDibit(dibit_p[m_symbolIndex]);
    }

    // only one slot in MS
    m_slot = DSDDMRSlot1;
    memcpy(&m_dsdDecoder->m_state.slot0light[4], "VOX", 3);
    m_voice1FrameCount = 0;
    m_dsdDecoder->m_voice1On = true;
    m_voice1EmbSig_dibitsIndex = 0;
    m_voice1EmbSig_OK = true;
}

void DSDDMR::processDataDibit(unsigned char dibit)
{
    int nextPartOff = IN_DIBITS(DMR_CACH_LEN);

    // CACH

    if (m_symbolIndex < nextPartOff)
    {
        if (m_burstType == DSDDMRBaseStation)
        {
            m_cachBits[m_cachInterleave[2*m_symbolIndex]]   = (dibit >> 1) & 1;
            m_cachBits[m_cachInterleave[2*m_symbolIndex+1]] = dibit & 1;

            if(m_symbolIndex == nextPartOff-1)
            {
                decodeCACH(m_cachBits);

    //            std::cerr << "DSDDMR::processDataDibit: start frame:"
    //                    << " slot: " << (int) m_slot
    //                    << " VC1: " << m_voice1FrameCount
    //                    << " VC2: " << m_voice2FrameCount << std::endl;
            }
        }
        return;
    }

    // data first half
    nextPartOff += IN_DIBITS(DMR_DATA_PART_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        // TODO
        return;
    }

    // Slot Type first half
    nextPartOff += IN_DIBITS(DMR_SLOT_TYPE_PART_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        m_slotTypePDU_dibits[m_symbolIndex - IN_DIBITS(DMR_CACH_LEN + DMR_DATA_PART_LEN)] = dibit;
        return;
    }

    // Sync or embedded signalling
    nextPartOff += IN_DIBITS(DMR_SYNC_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        // TODO
        return;
    }

    // Slot Type second half
    nextPartOff += IN_DIBITS(DMR_SLOT_TYPE_PART_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        int slotTypePDUOff = IN_DIBITS(DMR_SLOT_TYPE_PART_LEN) +
            m_symbolIndex - IN_DIBITS(DMR_CACH_LEN + DMR_DATA_PART_LEN +
                                      DMR_SLOT_TYPE_PART_LEN + DMR_SYNC_LEN);
        m_slotTypePDU_dibits[slotTypePDUOff] = dibit;

        if (m_symbolIndex == nextPartOff - 1)
        {
            processSlotTypePDU();
        }
        return;
    }

    // data second half
    nextPartOff += IN_DIBITS(DMR_DATA_PART_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        // TODO
        return;
    }
}

void DSDDMR::BasicPrivacyXOR(unsigned char *dibit, int index)
{
    if (m_dsdDecoder->m_opts.dmr_bp_key == 0)
        return; // Basic Privacy not used

    unsigned char key_number = m_dsdDecoder->m_opts.dmr_bp_key - 1;
    unsigned short key = BasicPrivacyKeys[key_number];

    const int key_bits = 16;
    int off = key_bits - ((index % 8) + 1) * 2;
    unsigned char out = *dibit ^ ((key >> off) & 3);
    *dibit = out;
}

void DSDDMR::processVoiceDibit(unsigned char dibit)
{
    int nextPartOff = IN_DIBITS(DMR_CACH_LEN);
    int CurOff = 0;
    // CACH

    if (m_symbolIndex < nextPartOff)
    {
        if (m_burstType == DSDDMRBaseStation)
        {
            m_cachBits[m_cachInterleave[2*m_symbolIndex]]   = (dibit >> 1) & 1;
            m_cachBits[m_cachInterleave[2*m_symbolIndex+1]] = dibit & 1;

            if(m_symbolIndex == nextPartOff-1)
            {
                decodeCACH(m_cachBits);

                if (m_cachOK)
                {
                    if (m_slot == DSDDMRSlot1) {
                        memcpy(&m_dsdDecoder->m_state.slot0light[4], "VOX", 3);
                    } else if (m_slot == DSDDMRSlot2) {
                        memcpy(&m_dsdDecoder->m_state.slot1light[4], "VOX", 3);
                    }
                }

    //            std::cerr << "DSDDMR::processVoiceDibit: start frame:"
    //                    << " slot: " << (int) m_slot
    //                    << " VC1: " << m_voice1FrameCount
    //                    << " VC2: " << m_voice2FrameCount << std::endl;
            }
        }
        return;
    }

    // voice frame 1
    CurOff = nextPartOff;
    nextPartOff += IN_DIBITS(DMR_VOCODER_FRAME_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        int mbeIndex = m_symbolIndex - IN_DIBITS(DMR_CACH_LEN);

        if (mbeIndex == 0)
        {
            w = rW;
            x = rX;
            y = rY;
            z = rZ;

            if (m_slot == DSDDMRSlot1) {
                memset((void *) m_dsdDecoder->m_mbeDVFrame1, 0, IN_BYTES(DMR_VOCODER_FRAME_LEN)); // initialize DVSI frame 1
            } else {
                memset((void *) m_dsdDecoder->m_mbeDVFrame2, 0, IN_BYTES(DMR_VOCODER_FRAME_LEN)); // initialize DVSI frame 2
            }
        }

        BasicPrivacyXOR(&dibit, mbeIndex);

        m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
        m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
        w++;
        x++;
        y++;
        z++;

        if (m_slot == DSDDMRSlot1) {
            storeSymbolDV(m_dsdDecoder->m_mbeDVFrame1, mbeIndex, dibit); // store dibit for DVSI hardware decoder
        } else { // it does not matter if CACH is undefined as it will be aborted later
            storeSymbolDV(m_dsdDecoder->m_mbeDVFrame2, mbeIndex, dibit); // store dibit for DVSI hardware decoder
        }

        if (mbeIndex == IN_DIBITS(DMR_VOCODER_FRAME_LEN) - 1)
        {
            if (m_slot == DSDDMRSlot1)
            {
                m_dsdDecoder->m_mbeDecoder1.processFrame(0, m_dsdDecoder->ambe_fr, 0);
                m_dsdDecoder->m_mbeDVReady1 = true; // Indicate that a DVSI frame is available
            }
            else if (m_slot == DSDDMRSlot2)
            {
                m_dsdDecoder->m_mbeDecoder2.processFrame(0, m_dsdDecoder->ambe_fr, 0);
                m_dsdDecoder->m_mbeDVReady2 = true; // Indicate that a DVSI frame is available
            }
        }
        return;
    }

    // voice frame 2 first half
    CurOff = nextPartOff;
    nextPartOff += IN_DIBITS(DMR_VOCODER_FRAME_LEN / 2);
    if (m_symbolIndex < nextPartOff)
    {
        int mbeIndex = m_symbolIndex - CurOff;

        if (mbeIndex == 0)
        {
            w = rW;
            x = rX;
            y = rY;
            z = rZ;

            memset((void *) m_mbeDVFrame, 0, IN_BYTES(DMR_VOCODER_FRAME_LEN)); // initialize DVSI frame
        }

        BasicPrivacyXOR(&dibit, mbeIndex);

        m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
        m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
        w++;
        x++;
        y++;
        z++;

        storeSymbolDV(m_mbeDVFrame, mbeIndex, dibit); // store dibit for DVSI hardware decoder
        return;
    }

    // EMB first half
    CurOff = nextPartOff;
    nextPartOff += IN_DIBITS(DMR_EMB_PART_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        m_emb_dibits[m_symbolIndex - CurOff] = dibit;
        return;
    }

    // Embedded signaling
    CurOff = nextPartOff;
    nextPartOff += IN_DIBITS(DMR_ES_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        m_voiceEmbSig_dibits[m_symbolIndex - CurOff] = dibit;
        return;
    }

    // EMB second half
    CurOff = nextPartOff;
    nextPartOff += IN_DIBITS(DMR_EMB_PART_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        m_emb_dibits[m_symbolIndex + IN_DIBITS(DMR_EMB_PART_LEN) - CurOff] = dibit;

        if (m_symbolIndex == nextPartOff - 1)
        {
            if ((m_slot == DSDDMRSlot1) && (m_voice1FrameCount > 0) && (m_voice1FrameCount < DMR_VOX_SUPERFRAME_LEN))
            {
                if (processEMB())
                {
                    if (processVoiceEmbeddedSignalling(m_voice1EmbSig_dibitsIndex, m_voice1EmbSigRawBits, m_voice1EmbSig_OK, m_slot1Addresses))
                    {
                        textVoiceEmbeddedSignalling(m_slot1Addresses, m_dsdDecoder->m_state.slot0light);
//                        std::cerr << "DSDDMR::processVoiceDibit: "
//                                << " source: " << m_slot1Addresses.m_source
//                                << " target: " << m_slot1Addresses.m_target
//                                << " group: " << m_slot1Addresses.m_group << std::endl;
                    }
                }
            }
            else if ((m_slot == DSDDMRSlot2) && (m_voice2FrameCount > 0) && (m_voice2FrameCount < DMR_VOX_SUPERFRAME_LEN))
            {
                if (processEMB())
                {
                    if (processVoiceEmbeddedSignalling(m_voice2EmbSig_dibitsIndex, m_voice2EmbSigRawBits, m_voice2EmbSig_OK, m_slot2Addresses))
                    {
                        textVoiceEmbeddedSignalling(m_slot2Addresses, m_dsdDecoder->m_state.slot1light);
//                        std::cerr << "DSDDMR::processVoiceDibit: "
//                                << " source: " << m_slot2Addresses.m_source
//                                << " target: " << m_slot2Addresses.m_target
//                                << " group: " << m_slot2Addresses.m_group << std::endl;
                    }
                }
            }
        }
        return;
    }

    // voice frame 2 second half
    CurOff = nextPartOff;
    nextPartOff += IN_DIBITS(DMR_VOCODER_FRAME_LEN / 2);
    if (m_symbolIndex < nextPartOff)
    {
        int mbeIndex = m_symbolIndex - (CurOff - IN_DIBITS(DMR_VOCODER_FRAME_LEN / 2));

        BasicPrivacyXOR(&dibit, mbeIndex);

        m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
        m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
        w++;
        x++;
        y++;
        z++;

        storeSymbolDV(m_mbeDVFrame, mbeIndex, dibit); // store dibit for DVSI hardware decoder

        if (mbeIndex == IN_DIBITS(DMR_VOCODER_FRAME_LEN) - 1)
        {
            if (m_slot == DSDDMRSlot1)
            {
                m_dsdDecoder->m_mbeDecoder1.processFrame(0, m_dsdDecoder->ambe_fr, 0);
                memcpy(m_dsdDecoder->m_mbeDVFrame1, m_mbeDVFrame, IN_BYTES(DMR_VOCODER_FRAME_LEN));
                m_dsdDecoder->m_mbeDVReady1 = true; // Indicate that a DVSI frame is available
            }
            else if (m_slot == DSDDMRSlot2)
            {
                m_dsdDecoder->m_mbeDecoder2.processFrame(0, m_dsdDecoder->ambe_fr, 0);
                memcpy(m_dsdDecoder->m_mbeDVFrame2, m_mbeDVFrame, IN_BYTES(DMR_VOCODER_FRAME_LEN));
                m_dsdDecoder->m_mbeDVReady2 = true; // Indicate that a DVSI frame is available
            }
        }
        return;
    }

    // voice frame 3
    CurOff = nextPartOff;
    nextPartOff += IN_DIBITS(DMR_VOCODER_FRAME_LEN);
    if (m_symbolIndex < nextPartOff)
    {
        int mbeIndex = m_symbolIndex - CurOff;

        BasicPrivacyXOR(&dibit, mbeIndex);

        if (mbeIndex == 0)
        {
            w = rW;
            x = rX;
            y = rY;
            z = rZ;

            if (m_slot == DSDDMRSlot1) {
                memset((void *) m_dsdDecoder->m_mbeDVFrame1, 0, IN_BYTES(DMR_VOCODER_FRAME_LEN)); // initialize DVSI frame 1
            } else {
                memset((void *) m_dsdDecoder->m_mbeDVFrame2, 0, IN_BYTES(DMR_VOCODER_FRAME_LEN)); // initialize DVSI frame 2
            }
        }

        m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
        m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
        w++;
        x++;
        y++;
        z++;

        if (m_slot == DSDDMRSlot1) {
            storeSymbolDV(m_dsdDecoder->m_mbeDVFrame1, mbeIndex, dibit); // store dibit for DVSI hardware decoder
        } else { // it does not matter if CACH is undefined as it will be aborted later
            storeSymbolDV(m_dsdDecoder->m_mbeDVFrame2, mbeIndex, dibit); // store dibit for DVSI hardware decoder
        }

        if (mbeIndex == IN_DIBITS(DMR_VOCODER_FRAME_LEN) - 1)
        {
            if (m_slot == DSDDMRSlot1)
            {
                m_dsdDecoder->m_mbeDecoder1.processFrame(0, m_dsdDecoder->ambe_fr, 0);
                m_dsdDecoder->m_mbeDVReady1 = true; // Indicate that a DVSI frame is available
            }
            else if (m_slot == DSDDMRSlot2)
            {
                m_dsdDecoder->m_mbeDecoder2.processFrame(0, m_dsdDecoder->ambe_fr, 0);
                m_dsdDecoder->m_mbeDVReady2 = true; // Indicate that a DVSI frame is available
            }
        }
        return;
    }
}

void DSDDMR::decodeCACH(unsigned char *cachBits)
{
    m_cachOK = true;

    if (m_continuation)
    {
        m_slot = (DSDDMRSlot) (((int) m_slot + 1) % 2);
//        std::cerr << "DSDDMR::decodeCACH: cach: " << " CC:"
//                << " at: " << m_cachSymbolIndex
//                << " slot: " << ((int) m_slot) << std::endl;
        m_continuation = false;
        m_cachSymbolIndex = 0; // restart counting
    }
    else
    {
        // Hamming (7,4) decode and store results if successful
        if (m_hamming_7_4.decode(cachBits)) // positive CACH information
        {
            unsigned int slotIndex = cachBits[1] & 1;
            m_dsdDecoder->m_state.currentslot = slotIndex; // FIXME: remove this when done with new voice processing

            if (slotIndex)
            {
                m_slotText = m_dsdDecoder->m_state.slot1light;
                m_dsdDecoder->m_state.slot0light[0] = ((cachBits[0] & 1) ? '*' : '.'); // the activity indicator is shifted by one slot
            }
            else
            {
                m_slotText = m_dsdDecoder->m_state.slot0light;
                m_dsdDecoder->m_state.slot1light[0] = ((cachBits[0] & 1) ? '*' : '.'); // the activity indicator is shifted by one slot
            }

            m_slot = (DSDDMRSlot) slotIndex;
            m_lcss = 2*cachBits[2] + cachBits[3];

//            std::cerr << "DSDDMR::decodeCACH: cach: " << " OK: at: " << m_cachSymbolIndex << " Slot: " << (int) cachBits[1] << " LCSS: " << (int) m_lcss << std::endl;

            m_cachSymbolIndex = 0; // restart counting
        }
        else
        {
            m_slot = DSDDMRSlotUndefined;
            m_cachOK = false;
//            std::cerr << "DSDDMR::decodeCACH: cach: " << " KO: at: " << m_cachSymbolIndex << std::endl;
        }
    }
}

void DSDDMR::processSlotTypePDU()
{
    unsigned char slotTypeBits[DMR_SLOT_TYPE_PART_LEN * 2];

    for (int i = 0; i < DMR_SLOT_TYPE_PART_LEN; i++)
    {
        slotTypeBits[2*i]     = (m_slotTypePDU_dibits[i] >> 1) & 1;
        slotTypeBits[2*i + 1] = m_slotTypePDU_dibits[i] & 1;
    }

    if (m_golay_20_8.decode(slotTypeBits))
    {
        m_colorCode = (slotTypeBits[0] << 3) + (slotTypeBits[1] << 2) + (slotTypeBits[2] << 1) + slotTypeBits[3];
        sprintf(&m_slotText[1], "%02d ", m_colorCode);

        unsigned int dataType = (slotTypeBits[4] << 3) + (slotTypeBits[5] << 2) + (slotTypeBits[6] << 1) + slotTypeBits[7];

        if (dataType > DMR_TYPES_COUNT)
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

bool DSDDMR::processEMB()
{
    unsigned char embBits[DMR_EMB_PART_LEN * 2];

    for (int i = 0; i < DMR_EMB_PART_LEN; i++)
    {
        embBits[2*i]     = (m_emb_dibits[i] >> 1) & 1;
        embBits[2*i + 1] = m_emb_dibits[i] & 1;
    }

    if (m_qr_16_7_6.decode(embBits))
    {
        m_colorCode = (embBits[0] << 3) + (embBits[1] << 2) + (embBits[2] << 1) + embBits[3];
        sprintf(&m_slotText[1], "%02d", m_colorCode);
        m_slotText[3] = ' ';
        m_lcss = (embBits[5] << 1) + embBits[6];
        return true;
    }
    else
    {
        return false;
    }
}

bool DSDDMR::processVoiceEmbeddedSignalling(int& voiceEmbSig_dibitsIndex,
        unsigned char *voiceEmbSigRawBits,
        bool& voiceEmbSig_OK,
        DMRAddresses& addresses)
{
    if (m_lcss != SingleLC_FirstCSBK) // skip RC
    {
        unsigned char parityCheck = 0;

        for (int i = 0; i < IN_DIBITS(DMR_ES_LEN); i++)
        {
            if (voiceEmbSig_dibitsIndex > 63) { // prevent segfault
                break;
            }

            int bit1Index = m_embSigInterleave[2*voiceEmbSig_dibitsIndex];
            int bit0Index = m_embSigInterleave[2*voiceEmbSig_dibitsIndex + 1];

            if ((i%4) == 0)
            {
                parityCheck = 0;
            }

            voiceEmbSigRawBits[bit1Index] = (1 & (m_voiceEmbSig_dibits[i] >> 1)); // bit 1
            voiceEmbSigRawBits[bit0Index] = (1 & m_voiceEmbSig_dibits[i]);        // bit 0
            parityCheck ^= voiceEmbSigRawBits[bit1Index];
            parityCheck ^= voiceEmbSigRawBits[bit0Index];

            if ((i%4) == 3)
            {
                if (parityCheck != 0)
                {
                    voiceEmbSig_OK = false;
                    break;
                }
            }

            voiceEmbSig_dibitsIndex++;
        }

        if (voiceEmbSig_dibitsIndex == 16*4) // BPTC matrix collected
        {
            if (m_hamming_16_11_4.decode(voiceEmbSigRawBits, 0, 7)) // TODO: 5 bit checksum
            {
                unsigned char flco = (voiceEmbSigRawBits[2] << 5)
                        + (voiceEmbSigRawBits[3] << 4)
                        + (voiceEmbSigRawBits[4] << 3)
                        + (voiceEmbSigRawBits[5] << 2)
                        + (voiceEmbSigRawBits[6] << 1)
                        + (voiceEmbSigRawBits[7]);
                addresses.m_group = (flco == 0);
                addresses.m_target = (voiceEmbSigRawBits[16*2 + 2] << 23) // (LC47)
                        + (voiceEmbSigRawBits[16*2 + 3] << 22)
                        + (voiceEmbSigRawBits[16*2 + 4] << 21)
                        + (voiceEmbSigRawBits[16*2 + 5] << 20)
                        + (voiceEmbSigRawBits[16*2 + 6] << 19)
                        + (voiceEmbSigRawBits[16*2 + 7] << 18)
                        + (voiceEmbSigRawBits[16*2 + 8] << 17)
                        + (voiceEmbSigRawBits[16*2 + 9] << 16) // 2:7
                        + (voiceEmbSigRawBits[16*3 + 0] << 15) // (LC39)
                        + (voiceEmbSigRawBits[16*3 + 1] << 14)
                        + (voiceEmbSigRawBits[16*3 + 2] << 13)
                        + (voiceEmbSigRawBits[16*3 + 3] << 12)
                        + (voiceEmbSigRawBits[16*3 + 4] << 11)
                        + (voiceEmbSigRawBits[16*3 + 5] << 10)
                        + (voiceEmbSigRawBits[16*3 + 6] << 9)
                        + (voiceEmbSigRawBits[16*3 + 7] << 8)  // 1:7
                        + (voiceEmbSigRawBits[16*3 + 8] << 7)  // (LC31)
                        + (voiceEmbSigRawBits[16*3 + 9] << 6)
                        + (voiceEmbSigRawBits[16*4 + 0] << 5)  // (LC29)
                        + (voiceEmbSigRawBits[16*4 + 1] << 4)
                        + (voiceEmbSigRawBits[16*4 + 2] << 3)
                        + (voiceEmbSigRawBits[16*4 + 3] << 2)
                        + (voiceEmbSigRawBits[16*4 + 4] << 1)
                        + (voiceEmbSigRawBits[16*4 + 5]);      // (LC24)
                addresses.m_source = (voiceEmbSigRawBits[16*4 + 6] << 23) // (LC23)
                        + (voiceEmbSigRawBits[16*4 + 7] << 22)
                        + (voiceEmbSigRawBits[16*4 + 8] << 21)
                        + (voiceEmbSigRawBits[16*4 + 9] << 20)
                        + (voiceEmbSigRawBits[16*5 + 0] << 19)
                        + (voiceEmbSigRawBits[16*5 + 1] << 18)
                        + (voiceEmbSigRawBits[16*5 + 2] << 17)
                        + (voiceEmbSigRawBits[16*5 + 3] << 16) // 2:7
                        + (voiceEmbSigRawBits[16*5 + 4] << 15) // (LC15)
                        + (voiceEmbSigRawBits[16*5 + 5] << 14)
                        + (voiceEmbSigRawBits[16*5 + 6] << 13)
                        + (voiceEmbSigRawBits[16*5 + 7] << 12)
                        + (voiceEmbSigRawBits[16*5 + 8] << 11)
                        + (voiceEmbSigRawBits[16*5 + 9] << 10)
                        + (voiceEmbSigRawBits[16*6 + 0] << 9)
                        + (voiceEmbSigRawBits[16*6 + 1] << 8)  // 1:7
                        + (voiceEmbSigRawBits[16*6 + 2] << 7)  // (LC7)
                        + (voiceEmbSigRawBits[16*6 + 3] << 6)
                        + (voiceEmbSigRawBits[16*6 + 4] << 5)
                        + (voiceEmbSigRawBits[16*6 + 5] << 4)
                        + (voiceEmbSigRawBits[16*6 + 6] << 3)
                        + (voiceEmbSigRawBits[16*6 + 7] << 2)
                        + (voiceEmbSigRawBits[16*6 + 8] << 1)
                        + (voiceEmbSigRawBits[16*6 + 9]);      // (LC0)

                return true; // we have a result
            }
            else
            {
                std::cerr << "DSDDMR::processVoiceEmbeddedSignalling: decode error" << std::endl;
                voiceEmbSig_OK = false;
            }
        }
    }

    return false; // no result yet or KO
}

void DSDDMR::storeSymbolDV(unsigned char *mbeFrame, int dibitindex, unsigned char dibit, bool invertDibit)
{
    if (m_dsdDecoder->m_mbelibEnable)
    {
        return;
    }

    if (invertDibit)
    {
        dibit = DSDcc::DSDSymbol::invert_dibit(dibit);
    }

    mbeFrame[dibitindex/4] |= (dibit << (6 - 2*(dibitindex % 4)));
}

void DSDDMR::textVoiceEmbeddedSignalling(DMRAddresses& addresses, char *slotText)
{
    sprintf(&slotText[8],  "%08u", addresses.m_source);
    sprintf(&slotText[18], "%08u", addresses.m_target);
    slotText[16] = '>';

    if (addresses.m_group) {
        slotText[17] = 'G';
    } else {
        slotText[17] = 'U';
    }
}

const char *DSDDMR::getSlot0Text() const
{
    return m_dsdDecoder->m_state.slot0light;
}

const char *DSDDMR::getSlot1Text() const
{
    return m_dsdDecoder->m_state.slot1light;
}

unsigned char DSDDMR::getColorCode() const
{
    return m_dsdDecoder->m_state.ccnum;
}

} // namespace DSDcc

