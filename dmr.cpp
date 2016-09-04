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
const int m_embSigInterleave[128] = {
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

// ========================================================================================

DSDDMR::DSDDMR(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbolIndex(0),
		m_cachSymbolIndex(0),
        m_burstType(DSDDMRBurstNone),
        m_slot(DSDDMRSlotUndefined),
        m_continuation(false),
        m_cachOK(false),
        m_lcss(0),
        m_colorCode(0),
        m_dataType(DSDDMRDataUnknown),
        m_voice1FrameCount(6),
        m_voice2FrameCount(6)
{
    m_slotText = m_dsdDecoder->m_state.slot0light;
    w = 0;
    x = 0;
    y = 0;
    z = 0;
}

DSDDMR::~DSDDMR()
{
}

void DSDDMR::initData(DSDDMRBurstType burstType)
{
//    std::cerr << "DSDDMR::initData" << std::endl;
    m_burstType = burstType;
    processDataFirstHalf();
}

void DSDDMR::initVoice(DSDDMRBurstType burstType)
{
//    std::cerr << "DSDDMR::initVoice" << std::endl;
    m_burstType = burstType;
    processVoiceFirstHalf();
}

void DSDDMR::processData()
{
    if (!m_cachOK)
    {
        m_slotText = m_dsdDecoder->m_state.slot0light;
        memcpy(m_dsdDecoder->m_state.slot0light, "/-- UNK", 7);
        m_dsdDecoder->resetFrameSync();
        return; // abort
    }

    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

    processDataDibit(dibit);

    if (m_symbolIndex == 144 - 1) // last dibit
    {
        if (m_slot == DSDDMRSlot1)
        {
            if (m_voice1FrameCount < 6)
            {
                if (m_voice2FrameCount < 6)
                {
//                    std::cerr << "DSDDMR::processData: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " voice continuation in slot 2" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 2
                    m_continuation = true;
                }
                else
                {
//                    std::cerr << "DSDDMR::processData: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " data continuation in slot 2" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRdata; // data continuation in slot 2
                    m_continuation = true;
                }
            }
            else
            {
                if (m_voice2FrameCount < 6)
                {
//                    std::cerr << "DSDDMR::processData: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " voice continuation in slot 2" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 2
                    m_continuation = true;
                }
                else
                {
//                    std::cerr << "DSDDMR::processData: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " back to search after slot 1" << std::endl;
                    m_dsdDecoder->resetFrameSync(); // back to sync
                    m_continuation = false;
                }
            }
        }
        else if (m_slot == DSDDMRSlot2)
        {
            if (m_voice2FrameCount < 6)
            {
                if (m_voice1FrameCount < 6)
                {
//                    std::cerr << "DSDDMR::processData: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " voice continuation in slot 1" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 1
                    m_continuation = true;
                }
                else
                {
//                    std::cerr << "DSDDMR::processData: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " data continuation in slot 1" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRdata; // data continuation in slot 1
                    m_continuation = true;
                }
            }
            else
            {
                if (m_voice1FrameCount < 6)
                {
//                    std::cerr << "DSDDMR::processData: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " voice continuation in slot 1" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 1
                    m_continuation = true;
                }
                else
                {
//                    std::cerr << "DSDDMR::processData: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " back to search after slot 2" << std::endl;
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

void DSDDMR::processVoice()
{
    if (!m_cachOK)
    {
        m_slotText = m_dsdDecoder->m_state.slot0light;
        memcpy(m_dsdDecoder->m_state.slot0light, "/-- UNK", 7);
        m_voice1FrameCount = 6;
        m_voice2FrameCount = 6;
        m_dsdDecoder->resetFrameSync();
        return; // abort
    }

	int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

	processVoiceDibit(dibit);

	if (m_symbolIndex == 144 - 1) // last dibit
	{
        if (m_slot == DSDDMRSlot1)
        {
            m_voice1FrameCount++;

            if (m_voice1FrameCount < 6)
            {
                if (m_voice2FrameCount < 6)
                {
//                    std::cerr << "DSDDMR::processVoice: slot: " << (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " voice continuation in slot 2" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 2
                    m_continuation = true;
                }
                else
                {
//                    std::cerr << "DSDDMR::processVoice: slot: " << (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " data continuation in slot 2" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRdata; // data continuation in slot 2
                    m_continuation = true;
                }
            }
            else
            {
                m_dsdDecoder->m_voice1On = false;

                if (m_voice2FrameCount < 6)
                {
//                    std::cerr << "DSDDMR::processVoice: slot: " << (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " voice continuation in slot 2" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 2
                    m_continuation = true;
                }
                else
                {
//                    std::cerr << "DSDDMR::processVoice: slot: " << (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " back to search after slot 1" << std::endl;
                    m_dsdDecoder->resetFrameSync(); // back to sync
                    m_continuation = false;
                }
            }
        }
        else if (m_slot == DSDDMRSlot2)
        {
            m_voice2FrameCount++;

            if (m_voice2FrameCount < 6)
            {
                if (m_voice1FrameCount < 6)
                {
//                    std::cerr << "DSDDMR::processVoice: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " voice continuation in slot 1" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 1
                }
                else
                {
//                    std::cerr << "DSDDMR::processVoice: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " data continuation in slot 1" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRdata; // data continuation in slot 1
                }
            }
            else
            {
                m_dsdDecoder->m_voice2On = false;

                if (m_voice1FrameCount < 6)
                {
//                    std::cerr << "DSDDMR::processVoice: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " voice continuation in slot 1" << std::endl;
                    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDMRvoice; // voice continuation in slot 1
                }
                else
                {
//                    std::cerr << "DSDDMR::processVoice: slot: "<< (int) m_slot+1 << " type: " << (int) m_dataType
//                            << " VF1: " << m_voice1FrameCount
//                            << " VF2: " << m_voice2FrameCount
//                            << " back to search after slot 2" << std::endl;
                    m_dsdDecoder->resetFrameSync(); // back to sync
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

void DSDDMR::processDataFirstHalf()
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(90+1);

    for (m_symbolIndex = 0; m_symbolIndex < 90; m_symbolIndex++, m_cachSymbolIndex++)
    {
        processDataDibit(dibit_p[m_symbolIndex]);
    }
}

void DSDDMR::processVoiceFirstHalf()
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(90+1);

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
        m_voice1FrameCount = 6;
        m_voice2FrameCount = 6;
        m_dsdDecoder->m_voice1On = false;
        m_dsdDecoder->m_voice2On = false;
        m_voice1EmbSig_OK = false;
        m_voice2EmbSig_OK = false;
    }

}

void DSDDMR::processDataDibit(unsigned char dibit)
{
	// CACH

	if (m_symbolIndex < 12)
	{
        m_cachBits[m_cachInterleave[2*m_symbolIndex]]   = (dibit >> 1) & 1;
        m_cachBits[m_cachInterleave[2*m_symbolIndex+1]] = dibit & 1;

        if(m_symbolIndex == 12-1)
        {
            decodeCACH(m_cachBits);
        }
	}

	// data first half

	else if (m_symbolIndex < 12 + 49)
	{
		// TODO
	}

	// Slot Type first half

	else if (m_symbolIndex < 12 + 49 + 5)
	{
		m_slotTypePDU_dibits[m_symbolIndex - (12 + 49)] = dibit;
	}

	// Sync or embedded signalling

	else if (m_symbolIndex < 12 + 49 + 5 + 24)
	{
		// TODO
	}

	// Slot Type second half

	else if (m_symbolIndex < 90 + 5)
    {
        m_slotTypePDU_dibits[5 + m_symbolIndex - 90] = dibit;

        if (m_symbolIndex == 90 + 5 - 1)
        {
            processSlotTypePDU();
        }
    }

    // data second half

    else if (m_symbolIndex < 90 + 5 + 49)
    {
    	// TODO
    }
}

void DSDDMR::processVoiceDibit(unsigned char dibit)
{
	// CACH

	if (m_symbolIndex < 12)
	{
        m_cachBits[m_cachInterleave[2*m_symbolIndex]]   = (dibit >> 1) & 1;
        m_cachBits[m_cachInterleave[2*m_symbolIndex+1]] = dibit & 1;

        if(m_symbolIndex == 12-1)
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
        }
	}

	// voice frame 1

	else if (m_symbolIndex < 12 + 36)
	{
		int mbeIndex = m_symbolIndex - 12;

		if (mbeIndex == 0)
		{
		    w = rW;
		    x = rX;
		    y = rY;
		    z = rZ;

	        if (m_slot == DSDDMRSlot1) {
	            memset((void *) m_dsdDecoder->m_mbeDVFrame1, 0, 9); // initialize DVSI frame 1
	        } else {
	            memset((void *) m_dsdDecoder->m_mbeDVFrame2, 0, 9); // initialize DVSI frame 2
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

		if (mbeIndex == 36 - 1)
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
	}

	// voice frame 2 first half

	else if (m_symbolIndex < 12 + 36 + 18)
	{
		int mbeIndex = m_symbolIndex - (12 + 36);

		if (mbeIndex == 0)
		{
			w = rW;
			x = rX;
			y = rY;
			z = rZ;

			memset((void *) m_mbeDVFrame, 0, 9); // initialize DVSI frame
		}

		m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
		m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
		w++;
		x++;
		y++;
		z++;

		storeSymbolDV(m_mbeDVFrame, mbeIndex, dibit); // store dibit for DVSI hardware decoder
	}

	// EMB first half

	else if (m_symbolIndex < 12 + 36 + 18 + 4)
	{
        m_emb_dibits[m_symbolIndex - (12 + 36 + 18)] = dibit;
	}

	// Embedded signaling

	else if (m_symbolIndex < 12 + 36 + 18 + 4 + 16)
	{
	    m_voiceEmbSig_dibits[m_symbolIndex - (12 + 36 + 18 + 4)] = dibit;
	}

	// EMB second half

	else if (m_symbolIndex < 12 + 36 + 18 + 4 + 16 + 4) // = 90
	{
        m_emb_dibits[m_symbolIndex - (12 + 36 + 18 + 4 + 16)] = dibit;

        if (m_symbolIndex == 12 + 36 + 18 + 4 + 16 + 4 - 1)
        {
            if (processEMB())
            {
                processVoiceEmbeddedSignalling();
            }
        }
	}

	// voice frame 2 second half

	else if (m_symbolIndex < 12 + 36 + 18 + 24 + 18)
	{
		int mbeIndex = m_symbolIndex - (12 + 36 + 24);

		m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
		m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
		w++;
		x++;
		y++;
		z++;

		storeSymbolDV(m_mbeDVFrame, mbeIndex, dibit); // store dibit for DVSI hardware decoder

		if (mbeIndex == 36 - 1)
		{
			if (m_slot == DSDDMRSlot1)
			{
				m_dsdDecoder->m_mbeDecoder1.processFrame(0, m_dsdDecoder->ambe_fr, 0);
				memcpy(m_dsdDecoder->m_mbeDVFrame1, m_mbeDVFrame, 9);
				m_dsdDecoder->m_mbeDVReady1 = true; // Indicate that a DVSI frame is available
			}
			else if (m_slot == DSDDMRSlot2)
			{
				m_dsdDecoder->m_mbeDecoder2.processFrame(0, m_dsdDecoder->ambe_fr, 0);
				memcpy(m_dsdDecoder->m_mbeDVFrame2, m_mbeDVFrame, 9);
				m_dsdDecoder->m_mbeDVReady2 = true; // Indicate that a DVSI frame is available
			}
		}
	}

	// voice frame 3

	else if (m_symbolIndex < 12 + 36 + 18 + 24 + 18 + 36)
	{
		int mbeIndex = m_symbolIndex - (12 + 36 + 18 + 24 + 18);

		if (mbeIndex == 0)
		{
		    w = rW;
		    x = rX;
		    y = rY;
		    z = rZ;

            if (m_slot == DSDDMRSlot1) {
                memset((void *) m_dsdDecoder->m_mbeDVFrame1, 0, 9); // initialize DVSI frame 1
            } else {
                memset((void *) m_dsdDecoder->m_mbeDVFrame2, 0, 9); // initialize DVSI frame 2
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

		if (mbeIndex == 36 - 1)
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
	}
}

void DSDDMR::decodeCACH(unsigned char *cachBits)
{
    m_cachOK = true;

    if (m_continuation)
    {
        m_slot = (DSDDMRSlot) (((int) m_slot + 1) % 2);
//        std::cerr << "DSDDMR::decodeCACH: CC: at: " << m_cachSymbolIndex << std::endl;
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
            }
            else
            {
                m_slotText = m_dsdDecoder->m_state.slot0light;
            }

            m_slotText[0] = ((cachBits[0] & 1) ? '*' : '.');
            m_slot = (DSDDMRSlot) slotIndex;
            m_lcss = 2*cachBits[2] + cachBits[3];

//            std::cerr << "DSDDMR::decodeCACH: OK: at: " << m_cachSymbolIndex << " Slot: " << (int) cachBits[1] << " LCSS: " << (int) m_lcss << std::endl;

            m_cachSymbolIndex = 0; // restart counting
        }
        else
        {
            m_slot = DSDDMRSlotUndefined;
            m_cachOK = false;
//            std::cerr << "DSDDMR::decodeCACH: KO: at: " << m_cachSymbolIndex << std::endl;
        }
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

bool DSDDMR::processEMB()
{
    unsigned char embBits[16];

    for (int i = 0; i < 8; i++)
    {
        embBits[2*i]     = (m_emb_dibits[i] >> 1) & 1;
        embBits[2*i + 1] = m_emb_dibits[i] & 1;
    }

    if (m_qr_16_7_6.decode(embBits))
    {
        m_colorCode = (embBits[0] << 3) + (embBits[1] << 2) + (embBits[2] << 1) + embBits[3];
        sprintf(&m_slotText[1], "%02d ", m_colorCode);
        m_lcss = (embBits[5] << 1) + embBits[6];
        return true;
    }
    else
    {
        return false;
    }
}

void DSDDMR::processVoiceEmbeddedSignalling()
{
    if (m_lcss != 0) // skip RC
    {
        unsigned char parityCheck;

        if ((m_slot == DSDDMRSlot1) && (m_voice1EmbSig_OK))
        {
            for (int i = 0; i < 16; i++)
            {
                int bit1Index = m_embSigInterleave[2*m_voice1EmbSig_dibitsIndex];
                int bit0Index = m_embSigInterleave[2*m_voice1EmbSig_dibitsIndex + 1];

                if ((i%4) == 0)
                {
                    parityCheck = 0;
                }

                m_voice1EmbSigRawBits[bit1Index] = (1 & (m_voiceEmbSig_dibits[i] >> 1)); // bit 1
                m_voice1EmbSigRawBits[bit0Index] = (1 & m_voiceEmbSig_dibits[i]);        // bit 0
                parityCheck ^= m_voice1EmbSigRawBits[bit1Index];
                parityCheck ^= m_voice1EmbSigRawBits[bit0Index];

                if ((i%4) == 3)
                {
                    if (parityCheck != 0)
                    {
                        m_voice1EmbSig_OK = false;
                        break;
                    }
                }

                m_voice1EmbSig_dibitsIndex++;
            }

            if (m_voice1EmbSig_dibitsIndex == 16*4) // BPTC matrix collected
            {

            }
        }
        else if ((m_slot == DSDDMRSlot2) && (m_voice2EmbSig_OK))
        {
            for (int i = 0; i < 16; i++)
            {
                int bit1Index = m_embSigInterleave[2*m_voice2EmbSig_dibitsIndex];
                int bit0Index = m_embSigInterleave[2*m_voice2EmbSig_dibitsIndex + 1];

                if ((i%4) == 0)
                {
                    parityCheck = 0;
                }

                m_voice2EmbSigRawBits[bit1Index] = (1 & (m_voiceEmbSig_dibits[i] >> 1)); // bit 1
                m_voice2EmbSigRawBits[bit0Index] = (1 & m_voiceEmbSig_dibits[i]);        // bit 0
                parityCheck ^= m_voice1EmbSigRawBits[bit1Index];
                parityCheck ^= m_voice1EmbSigRawBits[bit0Index];

                if ((i%4) == 3)
                {
                    if (parityCheck != 0)
                    {
                        m_voice2EmbSig_OK = false;
                        break;
                    }
                }

                m_voice2EmbSig_dibitsIndex++;
            }

            if (m_voice2EmbSig_dibitsIndex == 16*4) // BPTC matrix collected
            {

            }
        }
    }

    // complete embedded signaling test

    if (m_slot == DSDDMRSlot1)
    {

    }
    else if (m_slot == DSDDMRSlot2)
    {

    }
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


} // namespace DSDcc

