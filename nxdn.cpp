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
#include "nxdncrc.h"
#include "dsd_decoder.h"

namespace DSDcc
{

const char * DSDNXDN::nxdnRFChannelTypeText[4] = {
        "RC", //!< RCCH
        "RT", //!< RTCH
        "RD", //!< RDCH
        "RU"  //!< Unknown RF channel
};

const int DSDNXDN::SACCH::m_sacchInterleave[60] = {
    0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55,
    1, 6, 11, 16, 21, 26, 31, 36, 41, 46, 51, 56,
    2, 7, 12, 17, 22, 27, 32, 37, 42, 47, 52, 57,
    3, 8, 13, 18, 23, 28, 33, 38, 43, 48, 53, 58,
    4, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 59
};

const int DSDNXDN::SACCH::m_sacchPunctureList[12] = { 5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71 };

const int DSDNXDN::CACOutbound::m_cacInterleave[300] = {
    0, 25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275,
    1, 26, 51, 76, 101, 126, 151, 176, 201, 226, 251, 276,
    2, 27, 52, 77, 102, 127, 152, 177, 202, 227, 252, 277,
    3, 28, 53, 78, 103, 128, 153, 178, 203, 228, 253, 278,
    4, 29, 54, 79, 104, 129, 154, 179, 204, 229, 254, 279,
    5, 30, 55, 80, 105, 130, 155, 180, 205, 230, 255, 280,
    6, 31, 56, 81, 106, 131, 156, 181, 206, 231, 256, 281,
    7, 32, 57, 82, 107, 132, 157, 182, 207, 232, 257, 282,
    8, 33, 58, 83, 108, 133, 158, 183, 208, 233, 258, 283,
    9, 34, 59, 84, 109, 134, 159, 184, 209, 234, 259, 284,
    10, 35, 60, 85, 110, 135, 160, 185, 210, 235, 260, 285,
    11, 36, 61, 86, 111, 136, 161, 186, 211, 236, 261, 286,
    12, 37, 62, 87, 112, 137, 162, 187, 212, 237, 262, 287,
    13, 38, 63, 88, 113, 138, 163, 188, 213, 238, 263, 288,
    14, 39, 64, 89, 114, 139, 164, 189, 214, 239, 264, 289,
    15, 40, 65, 90, 115, 140, 165, 190, 215, 240, 265, 290,
    16, 41, 66, 91, 116, 141, 166, 191, 216, 241, 266, 291,
    17, 42, 67, 92, 117, 142, 167, 192, 217, 242, 267, 292,
    18, 43, 68, 93, 118, 143, 168, 193, 218, 243, 268, 293,
    19, 44, 69, 94, 119, 144, 169, 194, 219, 244, 269, 294,
    20, 45, 70, 95, 120, 145, 170, 195, 220, 245, 270, 295,
    21, 46, 71, 96, 121, 146, 171, 196, 221, 246, 271, 296,
    22, 47, 72, 97, 122, 147, 172, 197, 222, 247, 272, 297,
    23, 48, 73, 98, 123, 148, 173, 198, 223, 248, 273, 298,
    24, 49, 74, 99, 124, 149, 174, 199, 224, 249, 274, 299
};

const int DSDNXDN::CACOutbound::m_cacPunctureList[50] = {
    3, 11,    // 0
    17, 25,   // 1 14
    31, 39,   // 2 28
    45, 53,   // 3 42
    59, 67,   // 4 56
    73, 81,   // 5 70
    87, 95,   // 6 84
    101, 109, // 7 98
    115, 123, // 8 112
    129, 137, // 9 126
    143, 151, // 10 140
    157, 165, // 11 154
    171, 179, // 12 168
    185, 193, // 13 182
    199, 207, // 14 196
    213, 221, // 15 210
    227, 235, // 16 224
    241, 249, // 17 238
    255, 263, // 18 252
    269, 277, // 19 266
    283, 291, // 20 280
    297, 305, // 21 294
    311, 319, // 22 308
    325, 333, // 23 322
    339, 347  // 24 336
};

const int DSDNXDN::CACLong::m_cacInterleave[252] = {
    0, 21, 42, 63, 84, 105, 126, 147, 168, 189, 210, 231,
    1, 22, 43, 64, 85, 106, 127, 148, 169, 190, 211, 232,
    2, 23, 44, 65, 86, 107, 128, 149, 170, 191, 212, 233,
    3, 24, 45, 66, 87, 108, 129, 150, 171, 192, 213, 234,
    4, 25, 46, 67, 88, 109, 130, 151, 172, 193, 214, 235,
    5, 26, 47, 68, 89, 110, 131, 152, 173, 194, 215, 236,
    6, 27, 48, 69, 90, 111, 132, 153, 174, 195, 216, 237,
    7, 28, 49, 70, 91, 112, 133, 154, 175, 196, 217, 238,
    8, 29, 50, 71, 92, 113, 134, 155, 176, 197, 218, 239,
    9, 30, 51, 72, 93, 114, 135, 156, 177, 198, 219, 240,
    10, 31, 52, 73, 94, 115, 136, 157, 178, 199, 220, 241,
    11, 32, 53, 74, 95, 116, 137, 158, 179, 200, 221, 242,
    12, 33, 54, 75, 96, 117, 138, 159, 180, 201, 222, 243,
    13, 34, 55, 76, 97, 118, 139, 160, 181, 202, 223, 244,
    14, 35, 56, 77, 98, 119, 140, 161, 182, 203, 224, 245,
    15, 36, 57, 78, 99, 120, 141, 162, 183, 204, 225, 246,
    16, 37, 58, 79, 100, 121, 142, 163, 184, 205, 226, 247,
    17, 38, 59, 80, 101, 122, 143, 164, 185, 206, 227, 248,
    18, 39, 60, 81, 102, 123, 144, 165, 186, 207, 228, 249,
    19, 40, 61, 82, 103, 124, 145, 166, 187, 208, 229, 250,
    20, 41, 62, 83, 104, 125, 146, 167, 188, 209, 230, 251
};

const int DSDNXDN::CACLong::m_cacPunctureList[60] = {
    1, 7, 9, 11, 19,
    27, 33, 35, 37, 45,
    53, 59, 61, 63, 71,
    79, 85, 87, 89, 97,
    105, 111, 113, 115, 123,
    131, 137, 139, 141, 149,
    157, 163, 165, 167, 175,
    183, 189, 191, 193, 201,
    209, 215, 217, 219, 227,
    235, 241, 243, 245, 253,
    261, 267, 269, 271, 279,
    287, 293, 295, 297, 305
};

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

    m_rfChannel = NXDNRFCHUnknown;

    m_rfChannelStr[0] = '\0';
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

    if (m_symbolIndex < 6) {
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
	else if (m_symbolIndex < 8 + 174 - 1)
	{
        switch (m_rfChannel)
        {
        case NXDNRCCH:
            processRCCH(m_symbolIndex - 8, dibit);
            break;
        case NXDNRTCH:
            processRTCH(m_symbolIndex - 8, dibit);
            break;
        case NXDNRDCH:
            processRDCH(m_symbolIndex - 8, dibit);
            break;
        case NXDNRFCHUnknown:
        default:
            break; // do nothing
        }
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

    std::cerr << "DSDNXDN::processLICH:"
            << " rfChannelCode: " << m_lich.rfChannelCode
            << " fnChannelCode: " << m_lich.fnChannelCode
            << " optionCode: " << m_lich.optionCode
            << " direction: " << m_lich.direction
            << " parity: " << m_lich.parity
            << " m_lichEvenParity: " << m_lichEvenParity << std::endl;

    m_rfChannel = (m_lichEvenParity % 2) ? NXDNRFCHUnknown : (NXDNRFChannel) m_lich.rfChannelCode;
    memcpy(m_rfChannelStr, nxdnRFChannelTypeText[(int) m_rfChannel], 3);
}

void DSDNXDN::processRCCH(int index __attribute__((unused)), unsigned char dibit __attribute__((unused)))
{
}

void DSDNXDN::processRTCH(int index __attribute__((unused)), unsigned char dibit __attribute__((unused)))
{
}

void DSDNXDN::processRDCH(int index __attribute__((unused)), unsigned char dibit __attribute__((unused)))
{
}

DSDNXDN::FnChannel::FnChannel() :
    m_viterbi(2, Viterbi::Poly25y, true), // MSB first
    m_nbPuncture(0),
    m_rawSize(0),
    m_bufRaw(0),
    m_buf(0),
    m_interleave(0),
    m_punctureList(0)
{
    reset();
}

DSDNXDN::FnChannel::~FnChannel()
{}

void DSDNXDN::FnChannel::reset()
{
    m_index = 0;
}

void DSDNXDN::FnChannel::pushDibit(unsigned char dibit)
{
    m_bufRaw[m_interleave[m_index++]+m_nbPuncture] = dibit&1;
    m_bufRaw[m_interleave[m_index++]+m_nbPuncture] = dibit&2;
}

void DSDNXDN::FnChannel::unpuncture()
{
    if (m_nbPuncture == 0) {
        return;
    }

    int index, punctureIndex, i;
    bool punctureBit = false;

    for (index = 0, punctureIndex = 0, i = m_nbPuncture; i < m_rawSize; i++)
    {
        m_bufRaw[index++] = m_bufRaw[i];

        if (index == m_punctureList[punctureIndex])
        {
            m_bufRaw[index++] = punctureBit ? 1 : 0;
            punctureBit = !punctureBit;
            punctureIndex++;
        }
    }
}

DSDNXDN::SACCH::SACCH()
{
    m_rawSize = 72;
    m_nbPuncture = 12;
    m_bufRaw = m_sacchRaw;
    m_buf = m_sacch;
    m_interleave = m_sacchInterleave;
    m_punctureList = m_sacchPunctureList;
}

DSDNXDN::SACCH::~SACCH()
{}

void DSDNXDN::SACCH::decode()
{
    m_viterbi.decodeFromBits(m_sacch, m_sacchRaw, 72, 0);

    if (!CNXDNCRC::checkCRC6(m_sacch, 26))
    {
        std::cerr << "DSDNXDN::SACCH::decode: bad CRC" << std::endl;
        return;
    }
}

DSDNXDN::CACOutbound::CACOutbound()
{
    m_rawSize = 350;
    m_nbPuncture = 50;
    m_bufRaw = m_cacRaw;
    m_buf = m_cac;
    m_interleave = m_cacInterleave;
    m_punctureList = m_cacPunctureList;
}

DSDNXDN::CACOutbound::~CACOutbound()
{}

void DSDNXDN::CACOutbound::decode()
{
    m_viterbi.decodeFromBits(m_cac, m_cacRaw, 350, 0);

    if (!CNXDNCRC::checkCRC16(m_cac, 155))
    {
        std::cerr << "DSDNXDN::CACOutbound::decode: bad CRC" << std::endl;
        return;
    }
}

DSDNXDN::CACLong::CACLong()
{
    m_rawSize = 312;
    m_nbPuncture = 60;
    m_bufRaw = m_cacRaw;
    m_buf = m_cac;
    m_interleave = m_cacInterleave;
    m_punctureList = m_cacPunctureList;
}

DSDNXDN::CACLong::~CACLong()
{}

void DSDNXDN::CACLong::decode()
{
    m_viterbi.decodeFromBits(m_cac, m_cacRaw, 312, 0);

    if (!CNXDNCRC::checkCRC16(m_cac, 136))
    {
        std::cerr << "DSDNXDN::CACLong::decode: bad CRC" << std::endl;
        return;
    }
}

DSDNXDN::CACShort::CACShort()
{
    m_rawSize = 252;
    m_nbPuncture = 0;
    m_bufRaw = m_cacRaw;
    m_buf = m_cac;
    m_interleave = CACLong::m_cacInterleave;
}

DSDNXDN::CACShort::~CACShort()
{}

void DSDNXDN::CACShort::decode()
{
    m_viterbi.decodeFromBits(m_cac, m_cacRaw, 252, 0);

    if (!CNXDNCRC::checkCRC16(m_cac, 106))
    {
        std::cerr << "DSDNXDN::CACShort::decode: bad CRC" << std::endl;
        return;
    }
}

} // namespace DSDcc

