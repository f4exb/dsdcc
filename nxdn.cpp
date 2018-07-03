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
#include "nxdnconvolution.h"
#include "nxdncrc.h"
#include "dsd_decoder.h"

namespace DSDcc
{

/*
 * DMR AMBE interleave schedule
 */
// bit 1
const int DSDNXDN::rW[36] = {
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 2,
  0, 2, 0, 2, 0, 2,
  0, 2, 0, 2, 0, 2
};

const int DSDNXDN::rX[36] = {
  23, 10, 22, 9, 21, 8,
  20, 7, 19, 6, 18, 5,
  17, 4, 16, 3, 15, 2,
  14, 1, 13, 0, 12, 10,
  11, 9, 10, 8, 9, 7,
  8, 6, 7, 5, 6, 4
};

// bit 0
const int DSDNXDN::rY[36] = {
  0, 2, 0, 2, 0, 2,
  0, 2, 0, 3, 0, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3
};

const int DSDNXDN::rZ[36] = {
  5, 3, 4, 2, 3, 1,
  2, 0, 1, 13, 0, 12,
  22, 11, 21, 10, 20, 9,
  19, 8, 18, 7, 17, 6,
  16, 5, 15, 4, 14, 3,
  13, 2, 12, 1, 11, 0
};

const char * DSDNXDN::nxdnRFChannelTypeText[5] = {
        "RC", //!< RCCH
        "RT", //!< RTCH
        "RD", //!< RDCH
        "Rt", //!< RTCH-C
        "RU"  //!< Unknown RF channel
};

const int DSDNXDN::SACCH::m_Interleave[60] = {
    0, 12, 24, 36, 48,
    1, 13, 25, 37, 49,
    2, 14, 26, 38, 50,
    3, 15, 27, 39, 51,
    4, 16, 28, 40, 52,
    5, 17, 29, 41, 53,
    6, 18, 30, 42, 54,
    7, 19, 31, 43, 55,
    8, 20, 32, 44, 56,
    9, 21, 33, 45, 57,
    10, 22, 34, 46, 58,
    11, 23, 35, 47, 59,
};

const int DSDNXDN::SACCH::m_PunctureList[12] = { 5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71 };

const int DSDNXDN::CACOutbound::m_Interleave[300] = {
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180, 192, 204, 216, 228, 240, 252, 264, 276, 288,
    1, 13, 25, 37, 49, 61, 73, 85, 97, 109, 121, 133, 145, 157, 169, 181, 193, 205, 217, 229, 241, 253, 265, 277, 289,
    2, 14, 26, 38, 50, 62, 74, 86, 98, 110, 122, 134, 146, 158, 170, 182, 194, 206, 218, 230, 242, 254, 266, 278, 290,
    3, 15, 27, 39, 51, 63, 75, 87, 99, 111, 123, 135, 147, 159, 171, 183, 195, 207, 219, 231, 243, 255, 267, 279, 291,
    4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124, 136, 148, 160, 172, 184, 196, 208, 220, 232, 244, 256, 268, 280, 292,
    5, 17, 29, 41, 53, 65, 77, 89, 101, 113, 125, 137, 149, 161, 173, 185, 197, 209, 221, 233, 245, 257, 269, 281, 293,
    6, 18, 30, 42, 54, 66, 78, 90, 102, 114, 126, 138, 150, 162, 174, 186, 198, 210, 222, 234, 246, 258, 270, 282, 294,
    7, 19, 31, 43, 55, 67, 79, 91, 103, 115, 127, 139, 151, 163, 175, 187, 199, 211, 223, 235, 247, 259, 271, 283, 295,
    8, 20, 32, 44, 56, 68, 80, 92, 104, 116, 128, 140, 152, 164, 176, 188, 200, 212, 224, 236, 248, 260, 272, 284, 296,
    9, 21, 33, 45, 57, 69, 81, 93, 105, 117, 129, 141, 153, 165, 177, 189, 201, 213, 225, 237, 249, 261, 273, 285, 297,
    10, 22, 34, 46, 58, 70, 82, 94, 106, 118, 130, 142, 154, 166, 178, 190, 202, 214, 226, 238, 250, 262, 274, 286, 298,
    11, 23, 35, 47, 59, 71, 83, 95, 107, 119, 131, 143, 155, 167, 179, 191, 203, 215, 227, 239, 251, 263, 275, 287, 299,
};

const int DSDNXDN::CACOutbound::m_PunctureList[50] = {
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
    339, 347,  // 24 336
};

const int DSDNXDN::CACLong::m_Interleave[252] = {
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180, 192, 204, 216, 228, 240,
    1, 13, 25, 37, 49, 61, 73, 85, 97, 109, 121, 133, 145, 157, 169, 181, 193, 205, 217, 229, 241,
    2, 14, 26, 38, 50, 62, 74, 86, 98, 110, 122, 134, 146, 158, 170, 182, 194, 206, 218, 230, 242,
    3, 15, 27, 39, 51, 63, 75, 87, 99, 111, 123, 135, 147, 159, 171, 183, 195, 207, 219, 231, 243,
    4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124, 136, 148, 160, 172, 184, 196, 208, 220, 232, 244,
    5, 17, 29, 41, 53, 65, 77, 89, 101, 113, 125, 137, 149, 161, 173, 185, 197, 209, 221, 233, 245,
    6, 18, 30, 42, 54, 66, 78, 90, 102, 114, 126, 138, 150, 162, 174, 186, 198, 210, 222, 234, 246,
    7, 19, 31, 43, 55, 67, 79, 91, 103, 115, 127, 139, 151, 163, 175, 187, 199, 211, 223, 235, 247,
    8, 20, 32, 44, 56, 68, 80, 92, 104, 116, 128, 140, 152, 164, 176, 188, 200, 212, 224, 236, 248,
    9, 21, 33, 45, 57, 69, 81, 93, 105, 117, 129, 141, 153, 165, 177, 189, 201, 213, 225, 237, 249,
    10, 22, 34, 46, 58, 70, 82, 94, 106, 118, 130, 142, 154, 166, 178, 190, 202, 214, 226, 238, 250,
    11, 23, 35, 47, 59, 71, 83, 95, 107, 119, 131, 143, 155, 167, 179, 191, 203, 215, 227, 239, 251,
};

const int DSDNXDN::CACLong::m_PunctureList[60] = {
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

const int DSDNXDN::FACCH1::m_Interleave[144] = {
    0, 16, 32, 48, 64, 80, 96, 112, 128,
    1, 17, 33, 49, 65, 81, 97, 113, 129,
    2, 18, 34, 50, 66, 82, 98, 114, 130,
    3, 19, 35, 51, 67, 83, 99, 115, 131,
    4, 20, 36, 52, 68, 84, 100, 116, 132,
    5, 21, 37, 53, 69, 85, 101, 117, 133,
    6, 22, 38, 54, 70, 86, 102, 118, 134,
    7, 23, 39, 55, 71, 87, 103, 119, 135,
    8, 24, 40, 56, 72, 88, 104, 120, 136,
    9, 25, 41, 57, 73, 89, 105, 121, 137,
    10, 26, 42, 58, 74, 90, 106, 122, 138,
    11, 27, 43, 59, 75, 91, 107, 123, 139,
    12, 28, 44, 60, 76, 92, 108, 124, 140,
    13, 29, 45, 61, 77, 93, 109, 125, 141,
    14, 30, 46, 62, 78, 94, 110, 126, 142,
    15, 31, 47, 63, 79, 95, 111, 127, 143,
};

const int DSDNXDN::FACCH1::m_PunctureList[48] = {
    1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 65, 69, 73, 77,
    81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145,
    149, 153, 157, 161, 165, 169, 173, 177, 181, 185, 189,
};

const int DSDNXDN::UDCH::m_Interleave[348] = {
    0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180, 192, 204, 216, 228, 240, 252, 264, 276, 288, 300, 312, 324, 336,
    1, 13, 25, 37, 49, 61, 73, 85, 97, 109, 121, 133, 145, 157, 169, 181, 193, 205, 217, 229, 241, 253, 265, 277, 289, 301, 313, 325, 337,
    2, 14, 26, 38, 50, 62, 74, 86, 98, 110, 122, 134, 146, 158, 170, 182, 194, 206, 218, 230, 242, 254, 266, 278, 290, 302, 314, 326, 338,
    3, 15, 27, 39, 51, 63, 75, 87, 99, 111, 123, 135, 147, 159, 171, 183, 195, 207, 219, 231, 243, 255, 267, 279, 291, 303, 315, 327, 339,
    4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124, 136, 148, 160, 172, 184, 196, 208, 220, 232, 244, 256, 268, 280, 292, 304, 316, 328, 340,
    5, 17, 29, 41, 53, 65, 77, 89, 101, 113, 125, 137, 149, 161, 173, 185, 197, 209, 221, 233, 245, 257, 269, 281, 293, 305, 317, 329, 341,
    6, 18, 30, 42, 54, 66, 78, 90, 102, 114, 126, 138, 150, 162, 174, 186, 198, 210, 222, 234, 246, 258, 270, 282, 294, 306, 318, 330, 342,
    7, 19, 31, 43, 55, 67, 79, 91, 103, 115, 127, 139, 151, 163, 175, 187, 199, 211, 223, 235, 247, 259, 271, 283, 295, 307, 319, 331, 343,
    8, 20, 32, 44, 56, 68, 80, 92, 104, 116, 128, 140, 152, 164, 176, 188, 200, 212, 224, 236, 248, 260, 272, 284, 296, 308, 320, 332, 344,
    9, 21, 33, 45, 57, 69, 81, 93, 105, 117, 129, 141, 153, 165, 177, 189, 201, 213, 225, 237, 249, 261, 273, 285, 297, 309, 321, 333, 345,
    10, 22, 34, 46, 58, 70, 82, 94, 106, 118, 130, 142, 154, 166, 178, 190, 202, 214, 226, 238, 250, 262, 274, 286, 298, 310, 322, 334, 346,
    11, 23, 35, 47, 59, 71, 83, 95, 107, 119, 131, 143, 155, 167, 179, 191, 203, 215, 227, 239, 251, 263, 275, 287, 299, 311, 323, 335, 347,
};

const int DSDNXDN::UDCH::m_PunctureList[58] = {
    3, 11,
    17, 25,
    31, 39,
    45, 53,
    59, 67,
    73, 81,
    87, 95,
    101, 109,
    115, 123,
    129, 137,
    143, 151,
    157, 165,
    171, 179,
    185, 193,
    199, 207,
    213, 221,
    227, 235,
    241, 249,
    255, 263,
    269, 277,
    283, 291,
    297, 305,
    311, 319,
    325, 333,
    339, 347,
    353, 361,
    367, 375,
    381, 389,
    395, 403,
};

// NXDN Technical Specifications - NXDN TS 1-A Version 1.3 p.165
const unsigned char DSDNXDN::m_voiceTestPattern[36] = {
    3, 0, 3, 2, 2, 2, 2, 0, // CEA8 11 00 11 10 10 10 10 00
    3, 3, 3, 2, 2, 0, 0, 3, // FE83 11 11 11 10 10 00 00 11
    2, 2, 3, 0, 3, 0, 1, 0, // ACC4 10 10 11 00 11 00 01 00
    1, 1, 2, 0, 0, 2, 0, 0, // 5820 01 01 10 00 00 10 00 00
    0, 0, 2, 2,             // 0A 00 00 10 10
};

DSDNXDN::DSDNXDN(DSDDecoder *dsdDecoder) :
		m_dsdDecoder(dsdDecoder),
		m_state(NXDNFrame),
		m_pn(0xe4), // TS 1A v0103 section 4.6
		m_inSync(false),
		m_lichEvenParity(0),
		m_symbolIndex(0),
		m_swallowCount(0),
        w(0),
        x(0),
        y(0),
        z(0)
{
    memset(m_syncBuffer, 0, 11);
    memset(m_lichBuffer, 0, 8);

    m_rfChannel = NXDNRFCHUnknown;
    m_frameStructure = NXDNFSReserved;
    m_steal = NXDNStealReserved;
    m_ran = 0;
    m_idle = true;
    m_sourceId = 0;
    m_destinationId = 0;
    m_group = false;
    m_messageType = 0;
    m_locationId = 0;
    m_services = 0;
    m_fullRate = false;

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
        m_currentMessage.reset();
        m_inSync = true;
        m_fullRate = false;
        m_dsdDecoder->setMbeRate(DSDDecoder::DSDMBERate3600x2450);
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
        std::cerr << "DSDNXDN::process: unsupported state (end)" << std::endl;
        m_dsdDecoder->m_voice1On = false;
        m_dsdDecoder->resetFrameSync(); // end
        m_inSync = false;
    }
}

int DSDNXDN::unscrambleDibit(int dibit)
{
    //return dibit;
    return m_pn.getBit(m_symbolIndex) ? dibit ^ 2 : dibit; // apply PN scrambling. Inverting symbol is a XOR by 2 on the dibit.
}

void DSDNXDN::processFrame()
{
    int dibitRaw = m_dsdDecoder->m_dsdSymbol.getDibit();
    int dibit = unscrambleDibit(dibitRaw);

    // if (m_symbolIndex == 0) {
    //     std::cerr << "DSDNXDN::processFrame: start" << std::endl;
    // }

	if (m_symbolIndex < 8) // LICH info
	{
	    acquireLICH(dibit);
		m_symbolIndex++;

		if (m_symbolIndex == 8) {
			processLICH();
		}
	}
	else if (m_symbolIndex < 8 + 174)
	{
        switch (m_rfChannel)
        {
        case NXDNRCCH:
            processRCCH(m_symbolIndex - 8, dibit);
            break;
        case NXDNRTCH:
        case NXDNRDCH:
        case NXDNRTCHC:
            processRTDCH(m_symbolIndex - 8, dibit);
            break;
        case NXDNRFCHUnknown:
        default:
            break; // do nothing
        }
		m_symbolIndex++;
	}
	else // look for next 192 symbols frame...
	{
        // process first presumably sync symbol
        if ((dibitRaw == 0) || (dibitRaw == 1)) { // positives => 1 (+3)
            m_syncBuffer[0] = 1;
        } else { // negatives => 3 (-3)
            m_syncBuffer[0] = 3;
        }

		m_state = NXDNPostFrame; // look for next frame sync (FCH) or end
		m_symbolIndex = 1; // first sync symbol consumed already
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
        std::cerr << "DSDNXDN::processPostFrame: out of sync (end)" << std::endl;
        m_dsdDecoder->m_voice1On = false;
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
    } else
    {
        std::cerr << "DSDNXDN::processFSW: sync inconsistent (end)" << std::endl;
        m_dsdDecoder->m_voice1On = false;
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
        std::cerr << "DSDNXDN::processFSW: sync lost (end)" << std::endl;
        m_dsdDecoder->m_voice1On = false;
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

void DSDNXDN::acquireLICH(int dibit)
{
    m_lichBuffer[m_symbolIndex] = dibit >> 1; // conversion is a divide by 2

    if (m_symbolIndex < 6) {
        m_lichEvenParity += m_lichBuffer[m_symbolIndex];
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

    if (m_lichEvenParity % 2) // odd is wrong
    {
        m_rfChannel = NXDNRFCHUnknown;
        strcpy(m_rfChannelStr, "XX");
        m_dsdDecoder->m_voice1On = false;
        std::cerr << "DSDNXDN::processLICH: parity error" << std::endl;
        std::cerr << "DSDNXDN::processLICH:"
                << " rfChannelCode: " << m_lich.rfChannelCode
                << " fnChannelCode: " << m_lich.fnChannelCode
                << " optionCode: " << m_lich.optionCode
                << " direction: " << m_lich.direction
                << " parity: " << m_lich.parity
                << " m_lichEvenParity: " << m_lichEvenParity << std::endl;
    }
    else
    {
        m_rfChannel = (NXDNRFChannel) m_lich.rfChannelCode;
        memcpy(m_rfChannelStr, nxdnRFChannelTypeText[(int) m_rfChannel], 3);

        switch(m_rfChannel)
        {
        case NXDNRCCH:
            m_idle = false;
            if (m_lich.fnChannelCode == 0) {
                m_frameStructure = m_lich.direction ? NXDNFSCAC: NXDNFSReserved;
            } else if (m_lich.fnChannelCode == 1) {
                m_frameStructure = m_lich.direction ? NXDNFSReserved: NXDNFSCACLong;
            } else if (m_lich.fnChannelCode == 3) {
                m_frameStructure = m_lich.direction ? NXDNFSReserved: NXDNFSCACShort;
            } else {
                m_frameStructure = NXDNFSReserved;
            }
            break;
        case NXDNRTCH:
        case NXDNRDCH:
        case NXDNRTCHC:
            m_idle = false;
            if (m_lich.fnChannelCode == 0) {
                m_frameStructure = NXDNFSSACCH;
            } else if (m_lich.fnChannelCode == 1) {
                m_frameStructure = NXDNFSUDCH;
            } else if (m_lich.fnChannelCode == 2) {
                m_frameStructure = NXDNFSSACCHSup;
            } else {
                m_frameStructure = NXDNFSSACCHIdle;
                m_idle = true;
            }
            break;
        default:
            break;
        }

        if ((m_frameStructure == NXDNFSSACCH) || (m_frameStructure == NXDNFSSACCHSup))
        {
            m_steal = (NXDNSteal) m_lich.optionCode;
            m_dsdDecoder->m_voice1On = (m_steal != NXDNStealBoth);
        }
        else if (m_frameStructure == NXDNFSUDCH)
        {
            m_dsdDecoder->m_voice1On = false;

            if ((m_lich.optionCode == 0) || (m_lich.optionCode == 3)) {
                m_steal = (NXDNSteal) m_lich.optionCode;
            } else {
                m_steal = NXDNStealReserved;
            }
        }
        else
        {
            m_steal = NXDNStealReserved;
        }
    }
}

void DSDNXDN::processRCCH(int index, unsigned char dibit)
{
    switch(m_frameStructure)
    {
    case NXDNFSCAC: // CAC outbound
    {
        if (index == 0) {
            m_cac.reset();
        }

        if (index < 150) {
            m_cac.pushDibit(dibit);
        }

        if (index == 150)
        {
            m_cac.unpuncture();

            if (m_cac.decode())
            {
                m_ran = m_cac.getRAN();
                m_currentMessage.setFromCAC(&m_cac.getData()[1]);
                m_messageType = m_currentMessage.getMessageType();
                m_currentMessage.getSourceUnitId(m_sourceId);
                m_currentMessage.getDestinationGroupId(m_destinationId);
                m_currentMessage.isGroupCall(m_group);
                m_currentMessage.getLocationId(m_locationId);
                m_currentMessage.getServiceInformation(m_services);

                if (m_currentMessage.isFullRate(m_fullRate)) {
                    m_dsdDecoder->setMbeRate(isFullRate() ? DSDDecoder::DSDMBERate7200x4400 : DSDDecoder::DSDMBERate3600x2450);
                }

                if (m_cac.hasDualMessageFormat())
                {
                    m_currentMessage.setMessageIndex(1);
                    m_currentMessage.getSourceUnitId(m_sourceId);
                    m_currentMessage.getDestinationGroupId(m_destinationId);
                    m_currentMessage.isGroupCall(m_group);
                    m_currentMessage.getLocationId(m_locationId);
                    m_currentMessage.getServiceInformation(m_services);

                    if (m_currentMessage.isFullRate(m_fullRate)) {
                        m_dsdDecoder->setMbeRate(isFullRate() ? DSDDecoder::DSDMBERate7200x4400 : DSDDecoder::DSDMBERate3600x2450);
                    }

                    if (m_currentMessage.getAdjacentSitesInformation(m_adjacentSites, 1)) {
                        printAdjacentSites();
                    }

                    m_currentMessage.setMessageIndex(0);

                    if (m_currentMessage.getAdjacentSitesInformation(m_adjacentSites, 1)) {
                        printAdjacentSites();
                    }
                }
                else
                {
                    if (m_currentMessage.getAdjacentSitesInformation(m_adjacentSites, 3)) {
                        printAdjacentSites();
                    }
                }
            }
        }
    }
        break;
    case NXDNFSCACShort: // Short CAC
    {
        if (index == 0) {
            m_cacShort.reset();
        }

        if (index < 126) {
            m_cacShort.pushDibit(dibit);
        }

        if (index == 126)
        {
            m_cacShort.unpuncture();

            if (m_cacShort.decode())
            {
                m_ran = m_cacShort.getRAN();
                m_currentMessage.setFromCACShort(&m_cacShort.getData()[1]);
                m_messageType = m_currentMessage.getMessageType();
                m_currentMessage.getSourceUnitId(m_sourceId);
                m_currentMessage.getDestinationGroupId(m_destinationId);
                m_currentMessage.isGroupCall(m_group);
                m_currentMessage.getLocationId(m_locationId);
                m_currentMessage.getServiceInformation(m_services);

                if (m_currentMessage.isFullRate(m_fullRate)) {
                    m_dsdDecoder->setMbeRate(isFullRate() ? DSDDecoder::DSDMBERate7200x4400 : DSDDecoder::DSDMBERate3600x2450);
                }
            }
        }
    }
        break;
    case NXDNFSCACLong: // Long CAC
    {
        if (index == 0) {
            m_cacLong.reset();
        }

        if (index < 126) {
            m_cacLong.pushDibit(dibit);
        }

        if (index == 126)
        {
            m_cacLong.unpuncture();

            if (m_cacLong.decode())
            {
                m_ran = m_cacLong.getRAN();
                m_currentMessage.setFromCACLong(&m_cacLong.getData()[1]);
                m_messageType = m_currentMessage.getMessageType();
                m_currentMessage.getSourceUnitId(m_sourceId);
                m_currentMessage.getDestinationGroupId(m_destinationId);
                m_currentMessage.isGroupCall(m_group);
                m_currentMessage.getLocationId(m_locationId);
                m_currentMessage.getServiceInformation(m_services);

                if (m_currentMessage.isFullRate(m_fullRate)) {
                    m_dsdDecoder->setMbeRate(isFullRate() ? DSDDecoder::DSDMBERate7200x4400 : DSDDecoder::DSDMBERate3600x2450);
                }
            }
        }
    }
        break;
    default: // invalid
        break;
    }
}

void DSDNXDN::processRTDCH(int index, unsigned char dibit)
{
    if ((m_frameStructure == NXDNFSSACCH) || (m_frameStructure == NXDNFSSACCHSup))
    {
        if (index == 0) {
            m_sacch.reset();
        }

        if (index < 30) {
            m_sacch.pushDibit(dibit);
        }

        if (index == 30)
        {
            m_sacch.unpuncture();

            if (m_sacch.decode())
            {
                m_ran = m_sacch.getRAN();

                if ((m_sacch.getCountdown() == 0) && (m_sacch.getDecodeCount() == 0))
                {
                    m_currentMessage = m_sacch.getMessage();
                    m_messageType = m_currentMessage.getMessageType();
                    m_currentMessage.getSourceUnitId(m_sourceId);
                    m_currentMessage.getDestinationGroupId(m_destinationId);
                    m_currentMessage.isGroupCall(m_group);

                    if (m_currentMessage.isFullRate(m_fullRate)) {
                        m_dsdDecoder->setMbeRate(isFullRate() ? DSDDecoder::DSDMBERate7200x4400 : DSDDecoder::DSDMBERate3600x2450);
                    }
                }
            }
        }

        if (index >= 30)
        {
            int vindex = index - 30; // rebase index at start of Voice/FACCH frames

            if (m_steal == NXDNStealNone) // 4 voice frames EHR - 2 voice frames EFR
            {
                if (isFullRate()) {
                    processVoiceFrameEFR(vindex, dibit);
                } else {
                    processVoiceFrameEHR(vindex, dibit);
                }
            }
            else if (m_steal == NXDNSteal1) // FACCH1 then 2 voice frames EHR or 1 voice frame EFR
            {
                if (vindex < 72) {
                    processFACCH1(vindex, dibit);
                }
                else
                {
                    if (isFullRate()) {
                        processVoiceFrameEFR(vindex-72, dibit);
                    } else {
                        processVoiceFrameEHR(vindex-72, dibit);
                    }
                }
            }
            else if (m_steal == NXDNSteal2) // 2 voice frames EHR or 1 voice frame EFR then FACCH1
            {
                if (vindex < 72)
                {
                    if (isFullRate()) {
                        processVoiceFrameEFR(vindex, dibit);
                    } else {
                        processVoiceFrameEHR(vindex, dibit);
                    }
                }
                else {
                    processFACCH1(vindex-72, dibit);
                }
            }
            else if (m_steal == NXDNStealBoth) // All FACCH1
            {
                if (vindex < 72) {
                    processFACCH1(vindex, dibit);
                } else {
                    processFACCH1(vindex-72, dibit);
                }
            }
        }
    }
    else if (m_frameStructure == NXDNFSUDCH)
    {
        if (index == 0) {
            m_udch.reset();
        }

        if (index < 174) {
            m_udch.pushDibit(dibit);
        }

        if (index == 174)
        {
            m_udch.unpuncture();

            if (m_udch.decode())
            {
                m_ran = m_udch.getRAN();
                m_currentMessage.setFromFACCH2(&m_udch.getData()[1]);
                m_messageType = m_currentMessage.getMessageType();
                m_currentMessage.getSourceUnitId(m_sourceId);
                m_currentMessage.getDestinationGroupId(m_destinationId);
                m_currentMessage.isGroupCall(m_group);

                if (m_currentMessage.isFullRate(m_fullRate)) {
                    m_dsdDecoder->setMbeRate(isFullRate() ? DSDDecoder::DSDMBERate7200x4400 : DSDDecoder::DSDMBERate3600x2450);
                }

                if (m_steal == NXDNStealBoth) // This is a FACCH2
                {
                    if (m_currentMessage.getAdjacentSitesInformation(m_adjacentSites, 4)) {
                        printAdjacentSites();
                    }
                }
            }
        }
    }
    // Do nothing if SACCH with idle status
}

void DSDNXDN::processFACCH1(int index, unsigned char dibit)
{
    if (index == 0) {
        m_facch1.reset();
    }

    if (index < 72) {
        m_facch1.pushDibit(dibit);
    }

    if (index == 72-1)
    {
        m_facch1.unpuncture();

        if (m_facch1.decode())
        {
            m_currentMessage.setFromFACCH1(m_facch1.getData());
            m_messageType = m_currentMessage.getMessageType();
            m_currentMessage.getSourceUnitId(m_sourceId);
            m_currentMessage.getDestinationGroupId(m_destinationId);
            m_currentMessage.isGroupCall(m_group);

            if (m_currentMessage.isFullRate(m_fullRate)) {
                m_dsdDecoder->setMbeRate(isFullRate() ? DSDDecoder::DSDMBERate7200x4400 : DSDDecoder::DSDMBERate3600x2450);
            }

            if (m_currentMessage.getAdjacentSitesInformation(m_adjacentSites, 1)) {
                printAdjacentSites();
            }
        }

        m_facch1.reset();
    }
}

DSDNXDN::FnChannel::FnChannel() :
    m_nbPuncture(0),
    m_rawSize(0),
    m_bufRaw(0),
    m_bufTmp(0),
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
    m_bufRaw[m_interleave[m_index++]] = (dibit&2)>>1;
    m_bufRaw[m_interleave[m_index++]] = dibit&1;
}

void DSDNXDN::FnChannel::unpuncture()
{
    if (m_nbPuncture == 0) {
        return;
    }

    int index, punctureIndex, i;

    for (index = 0, punctureIndex = 0, i = 0; i < m_rawSize; i++)
    {
        if (index == m_punctureList[punctureIndex])
        {
            m_bufTmp[index++] = 1;
            punctureIndex++;
        }

        m_bufTmp[index++] = m_bufRaw[i]<<1; // 0->0, 1->2
    }

    for (int i=0; i<8; i++) {
        m_bufTmp[index++] = 0;
    }
}

DSDNXDN::SACCH::SACCH()
{
    m_rawSize = 60;
    m_nbPuncture = 12;
    m_bufRaw = m_sacchRaw;
    m_bufTmp = m_temp;
    m_interleave = m_Interleave;
    m_punctureList = m_PunctureList;
    m_message.reset();
    m_decodeCount = 0;
}

DSDNXDN::SACCH::~SACCH()
{}

bool DSDNXDN::SACCH::decode()
{
    CNXDNConvolution conv;
    conv.start();
    int n = 0;

    for (unsigned int i = 0U; i < 40U; i++)
    {
        uint8_t s0 = m_temp[n++];
        uint8_t s1 = m_temp[n++];

        conv.decode(s0, s1);
    }

    conv.chainback(m_data, 36U);

    if (!CNXDNCRC::checkCRC6(m_data, 26U))
    {
        std::cerr << "DSDNXDN::SACCH::decode: bad CRC" << std::endl;

        if (m_decodeCount >= 0) {
            m_decodeCount = -1;
        }

        return false;
    }
    else
    {
        if (getCountdown() == 3) {
            m_decodeCount = 3;
        } else {
            m_decodeCount--;
        }

        m_message.setFromSACCH(3-getCountdown(), &m_data[1]);
        return true;
    }
}

unsigned char DSDNXDN::SACCH::getRAN() const
{
    return m_data[0U] & 0x3FU;
}

unsigned char DSDNXDN::SACCH::getCountdown() const
{
    return (m_data[0U] >> 6) & 0x03U;
}


DSDNXDN::CACOutbound::CACOutbound()
{
    m_rawSize = 300;
    m_nbPuncture = 50;
    m_bufRaw = m_cacRaw;
    m_bufTmp = m_temp;
    m_interleave = m_Interleave;
    m_punctureList = m_PunctureList;
}

DSDNXDN::CACOutbound::~CACOutbound()
{}

bool DSDNXDN::CACOutbound::decode()
{
    CNXDNConvolution conv;
    conv.start();
    int n = 0;

    for (unsigned int i = 0U; i < 179U; i++)
    {
        uint8_t s0 = m_temp[n++];
        uint8_t s1 = m_temp[n++];

        conv.decode(s0, s1);
    }

    conv.chainback(m_data, 175U);

    if (!CNXDNCRC::checkCRC16(m_data, 155))
    {
        std::cerr << "DSDNXDN::CACOutbound::decode: bad CRC" << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

unsigned char DSDNXDN::CACOutbound::getRAN() const
{
    return m_data[0U] & 0x3FU;
}

bool DSDNXDN::CACOutbound::isHeadOfSuperframe() const
{
    return (m_data[0U] & 0x80U) == 0x80U;
}

bool DSDNXDN::CACOutbound::hasDualMessageFormat() const
{
    return (m_data[0U] & 0x40U) == 0x40U;
}

unsigned char DSDNXDN::CACOutbound::getMessageType() const
{
    return m_data[1U] & 0x3FU;
}

DSDNXDN::CACLong::CACLong()
{
    m_rawSize = 252;
    m_nbPuncture = 60;
    m_bufRaw = m_cacRaw;
    m_bufTmp = m_temp;
    m_interleave = m_Interleave;
    m_punctureList = m_PunctureList;
}

DSDNXDN::CACLong::~CACLong()
{}

bool DSDNXDN::CACLong::decode()
{
    CNXDNConvolution conv;
    conv.start();
    int n = 0;

    for (unsigned int i = 0U; i < 160U; i++)
    {
        uint8_t s0 = m_temp[n++];
        uint8_t s1 = m_temp[n++];

        conv.decode(s0, s1);
    }

    conv.chainback(m_data, 156U);

    if (!CNXDNCRC::checkCRC16(m_data, 136))
    {
        std::cerr << "DSDNXDN::CACLong::decode: bad CRC" << std::endl;
        return false;
    }
    else
    {
        std::cerr << "DSDNXDN::CACLong::decode: CRC OK" << std::endl;
        return true;
    }
}

unsigned char DSDNXDN::CACLong::getRAN() const
{
    return m_data[0U] & 0x3FU;
}

DSDNXDN::CACShort::CACShort()
{
    m_rawSize = 252;
    m_nbPuncture = 0;
    m_bufRaw = m_cacRaw;
    m_bufTmp = m_temp;
    m_interleave = CACLong::m_Interleave;
}

DSDNXDN::CACShort::~CACShort()
{}

bool DSDNXDN::CACShort::decode()
{
    CNXDNConvolution conv;
    conv.start();
    int n = 0;

    for (unsigned int i = 0U; i < 130U; i++)
    {
        uint8_t s0 = m_temp[n++];
        uint8_t s1 = m_temp[n++];

        conv.decode(s0, s1);
    }

    conv.chainback(m_data, 126U);

    if (!CNXDNCRC::checkCRC16(m_data, 106))
    {
        std::cerr << "DSDNXDN::CACShort::decode: bad CRC" << std::endl;
        return false;
    }
    else
    {
        std::cerr << "DSDNXDN::CACShort::decode: CRC OK" << std::endl;
        return true;
    }
}

unsigned char DSDNXDN::CACShort::getRAN() const
{
    return m_data[0U] & 0x3FU;
}

DSDNXDN::FACCH1::FACCH1()
{
    m_rawSize = 144;
    m_nbPuncture = 48;
    m_bufRaw = m_facch1Raw;
    m_bufTmp = m_temp;
    m_interleave = m_Interleave;
    m_punctureList = m_PunctureList;
}

DSDNXDN::FACCH1::~FACCH1()
{}

bool DSDNXDN::FACCH1::decode()
{
    CNXDNConvolution conv;
    conv.start();
    int n = 0;

    for (unsigned int i = 0U; i < 100U; i++)
    {
        uint8_t s0 = m_temp[n++];
        uint8_t s1 = m_temp[n++];

        conv.decode(s0, s1);
    }

    conv.chainback(m_data, 96U);

    if (!CNXDNCRC::checkCRC12(m_data, 80))
    {
        std::cerr << "DSDNXDN::FACCH1::decode: bad CRC" << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

DSDNXDN::UDCH::UDCH()
{
    m_rawSize = 348;
    m_nbPuncture = 58;
    m_bufRaw = m_udchRaw;
    m_bufTmp = m_temp;
    m_interleave = m_Interleave;
    m_punctureList = m_PunctureList;
}

DSDNXDN::UDCH::~UDCH()
{}

bool DSDNXDN::UDCH::decode()
{
    CNXDNConvolution conv;
    conv.start();
    int n = 0;

    for (unsigned int i = 0U; i < 207U; i++)
    {
        uint8_t s0 = m_temp[n++];
        uint8_t s1 = m_temp[n++];

        conv.decode(s0, s1);
    }

    conv.chainback(m_data, 203U);

    if (!CNXDNCRC::checkCRC15(m_data, 184))
    {
        std::cerr << "DSDNXDN::UDCH::decode: bad CRC" << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

unsigned char DSDNXDN::UDCH::getRAN() const
{
    return m_data[0U] & 0x3FU;
}

unsigned char DSDNXDN::UDCH::getStructure() const
{
    return (m_data[0U] >> 6) & 0x03U;
}

void DSDNXDN::processVoiceTest(int symbolIndex)
{
    processVoiceFrameEHR(symbolIndex, m_voiceTestPattern[symbolIndex%36]);
}

void DSDNXDN::processVoiceFrameEHR(int symbolIndex, int dibit)
{
    // if (symbolIndex % 36 == 0) {
    //     std::cerr << "DSDNXDN::processVoiceFrame: start: " << m_symbolIndex << " " << symbolIndex << std::endl;
    // }

    if ((symbolIndex == 0) && (m_dsdDecoder->m_opts.errorbars == 1))
    {
        m_dsdDecoder->getLogger().log("\nMBE: ");
    }

    if (symbolIndex % 36 == 0)
    {
        w = rW;
        x = rX;
        y = rY;
        z = rZ;
        memset((void *) m_dsdDecoder->m_mbeDVFrame1, 0, 9); // initialize DVSI frame
    }

    m_dsdDecoder->ambe_fr[*w][*x] = (1 & (dibit >> 1)); // bit 1
    m_dsdDecoder->ambe_fr[*y][*z] = (1 & dibit);        // bit 0
    w++;
    x++;
    y++;
    z++;

    storeSymbolDV(symbolIndex % 36, dibit); // store dibit for DVSI hardware decoder

    if (symbolIndex % 36 == 35)
    {
        m_dsdDecoder->m_mbeDecoder1.processFrame(0, m_dsdDecoder->ambe_fr, 0);
        m_dsdDecoder->m_mbeDVReady1 = true; // Indicate that a DVSI frame is available

        if (m_dsdDecoder->m_opts.errorbars == 1)
        {
            m_dsdDecoder->getLogger().log(".");
        }
    }
}

void DSDNXDN::processVoiceFrameEFR(int symbolIndex, int dibit)
{
    if ((symbolIndex == 0) && (m_dsdDecoder->m_opts.errorbars == 1))
    {
        m_dsdDecoder->getLogger().log("\nMBE: ");
    }

    storeSymbolDV(symbolIndex % 72, dibit);

    if (symbolIndex % 72 == 71)
    {
        m_dsdDecoder->m_mbeDVReady1 = true; // Indicate that a DVSI frame is available

        if (m_dsdDecoder->m_opts.errorbars == 1)
        {
            m_dsdDecoder->getLogger().log(".");
        }
    }
}

void DSDNXDN::storeSymbolDV(int dibitindex, unsigned char dibit, bool invertDibit)
{
    if (m_dsdDecoder->m_mbelibEnable)
    {
        return;
    }

    if (invertDibit)
    {
        dibit = DSDcc::DSDSymbol::invert_dibit(dibit);
    }

    m_dsdDecoder->m_mbeDVFrame1[dibitindex/4] |= (dibit << (6 - 2*(dibitindex % 4)));
}

void DSDNXDN::resetAdjacentSites()
{
    for (int i=0; i<16; i++)
    {
        m_adjacentSites[i].m_channelNumber = 0;
        m_adjacentSites[i].m_locationId = 0;
        m_adjacentSites[i].m_siteNumber = 0;
    }
}

void DSDNXDN::printAdjacentSites()
{
    for (int i=0; i<16; i++)
    {
        if ( m_adjacentSites[i].m_siteNumber == 0) {
            continue;
        }

        std::cerr << "DSDNXDN::printAdjacentSites:"
            << " site: " << m_adjacentSites[i].m_siteNumber
            << " channel: " << m_adjacentSites[i].m_channelNumber
            << " location: " << std::hex << m_adjacentSites[i].m_locationId << std::endl;
    }
}

bool DSDNXDN::isFullRate() const
{
    return (m_dsdDecoder->getDataRate() == DSDDecoder::DSDRate4800) &&  m_fullRate;
}


} // namespace DSDcc

