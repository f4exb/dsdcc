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
        m_burstType(DSDDMRBurstNone),
        m_slot(DSDDMRSlotUndefined),
        m_prevSlot(DSDDMRSlotUndefined),
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
    m_burstType = burstType;
    processDataFirstHalf();
}

void DSDDMR::initVoice(DSDDMRBurstType burstType)
{
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

    if (m_symbolIndex < 90 + 5)
    {
        m_slotTypePDU_dibits[5 + m_symbolIndex - 90] = dibit;

        if (m_symbolIndex == 90 + 5 - 1)
        {
            processSlotTypePDU();
        }
    }
    else if (m_symbolIndex == 90 + 5 + 49) // last dibit
    {
        m_prevSlot = m_slot;
        m_dsdDecoder->resetFrameSync(); // end TODO: continuation if super frame on going
    }

    m_symbolIndex++;
}

void DSDDMR::processVoice()
{
	int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol

	processVoiceDibit(dibit);

	if (m_symbolIndex == 144 - 1) // last dibit
	{
        m_prevSlot = m_slot;
        m_dsdDecoder->resetFrameSync(); // end TODO: continuation if super frame on going
	}

	m_symbolIndex++;
}

void DSDDMR::processDataFirstHalf()
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(90+1);
    m_cachOK = true;

    if (m_burstType == DSDDMRBaseStation) // CACH is for base station only
    {
        if (!processCACH(dibit_p))
        {
            m_cachOK = false; // cannot determine slot => sync lost
        }
    }

    dibit_p += 12; // move after either CACH or garbage

    dibit_p += 49; // move after first half info block

    memcpy((void *) m_slotTypePDU_dibits, (const void *) dibit_p, 5);

    dibit_p += 5;  // move after first half slot type PDU

    dibit_p += 24; // move after SYNC or embedded signaling

    m_symbolIndex = 90;
}

void DSDDMR::processVoiceFirstHalf()
{
    unsigned char *dibit_p = m_dsdDecoder->m_dsdSymbol.getDibitBack(90+1);

    for (m_symbolIndex = 0; m_symbolIndex < 90; m_symbolIndex++)
    {
    	if (!processVoiceDibit(dibit_p[m_symbolIndex])) {
    		break; // abort
    	}
    }
}

bool DSDDMR::processDataDibit(unsigned char dibit)
{
	// CACH

	unsigned char cachBits[24];

	if ((m_symbolIndex < 12) && (m_burstType == DSDDMRBaseStation)) // CACH is for base station only
	{
        cachBits[m_cachInterleave[2*m_symbolIndex]]   = (dibit >> 1) & 1;
        cachBits[m_cachInterleave[2*m_symbolIndex+1]] = dibit & 1;

        if(m_symbolIndex == 12-1)
        {
            if (!decodeCACH(cachBits)) // unrecoverable sync error
            {
                return false; // abort
            }
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
		m_slotTypePDU_dibits[m_symbolIndex - (12 + 49)];
	}

	// Sync or embedded signalling

	else if (m_symbolIndex < 12 + 49 + 5 + 24)
	{
		// TODO
	}

	// Slot Type second half

    if (m_symbolIndex < 90 + 5)
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

bool DSDDMR::processVoiceDibit(unsigned char dibit)
{
	// CACH

	unsigned char cachBits[24];

	if ((m_symbolIndex < 12) && (m_burstType == DSDDMRBaseStation)) // CACH is for base station only
	{
        cachBits[m_cachInterleave[2*m_symbolIndex]]   = (dibit >> 1) & 1;
        cachBits[m_cachInterleave[2*m_symbolIndex+1]] = dibit & 1;

        if(m_symbolIndex == 12-1)
        {
            if (!decodeCACH(cachBits)) // unrecoverable sync error
            {
                return false; // abort
            }
        }
	}

	// voice frame 1

	else if (m_symbolIndex < 12 + 36)
	{
		unsigned char *mbeFrame;
		int mbeIndex = m_symbolIndex - 12;

		if (mbeIndex == 0)
		{
		    if (m_slot == DSDDMRSlot1) {
		        mbeFrame = m_dsdDecoder->m_mbeDVFrame1;
		    } else if (m_slot == DSDDMRSlot2) {
		        mbeFrame = m_dsdDecoder->m_mbeDVFrame2;
		    }

		    w = rW;
		    x = rX;
		    y = rY;
		    z = rZ;

		    memset((void *) mbeFrame, 0, 9); // initialize DVSI frame
		}

		m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
		m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
		w++;
		x++;
		y++;
		z++;

		storeSymbolDV(mbeFrame, mbeIndex, dibit); // store dibit for DVSI hardware decoder

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
		// TODO
	}

	// Embedded signaling

	else if (m_symbolIndex < 12 + 36 + 18 + 4 + 16)
	{

	}

	// EMB second half

	else if (m_symbolIndex < 12 + 36 + 18 + 4 + 16 + 4)
	{

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
		unsigned char *mbeFrame;
		int mbeIndex = m_symbolIndex - 12 + 36 + 18 + 24 + 18;

		if (mbeIndex == 0)
		{
		    if (m_slot == DSDDMRSlot1) {
		        mbeFrame = m_dsdDecoder->m_mbeDVFrame1;
		    } else if (m_slot == DSDDMRSlot2) {
		        mbeFrame = m_dsdDecoder->m_mbeDVFrame2;
		    }

		    w = rW;
		    x = rX;
		    y = rY;
		    z = rZ;

		    memset((void *) mbeFrame, 0, 9); // initialize DVSI frame
		}

		m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
		m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
		w++;
		x++;
		y++;
		z++;

		storeSymbolDV(mbeFrame, mbeIndex, dibit); // store dibit for DVSI hardware decoder

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

	return true;
}

bool DSDDMR::processCACH(unsigned char *dibit_p)
{
    unsigned char cachBits[24];

    for (int i = 0; i < 12; i++)
    {
        cachBits[m_cachInterleave[2*i]]   = (dibit_p[i] >> 1) & 1;
        cachBits[m_cachInterleave[2*i+1]] = dibit_p[i] & 1;
    }

    return decodeCACH(cachBits);
}

bool DSDDMR::decodeCACH(unsigned char *cachBits)
{
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
        m_slot = (DSDDMRSlot) slotIndex;
        m_lcss = 2*cachBits[2] + cachBits[3];

        if (m_prevSlot == m_slot) // conflict with previous slot
        {
            m_prevSlot = DSDDMRSlotUndefined; // break continuation
        }
//        std::cerr << "DSDDMR::processCACH: OK: Slot: " << (int) cachBits[1] << " LCSS: " << (int) m_lcss << std::endl;
    }
    else
    {
        m_slot = DSDDMRSlotUndefined;

        if (m_prevSlot == DSDDMRSlotUndefined) // sync lost
        {
            return false;
        }
        else
        {
            unsigned int slotIndex = (((int) m_prevSlot + 1) % 2);

            if (slotIndex)
            {
                m_slotText = m_dsdDecoder->m_state.slot1light;
            }
            else
            {
                m_slotText = m_dsdDecoder->m_state.slot0light;
            }

            m_slot = (DSDDMRSlot) slotIndex;
            m_slotText[0] = ':';
        }
//        std::cerr << "DSDDMR::processCACH: KO" << std::endl;
    }

    return true;
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

