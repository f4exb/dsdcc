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

#include "ysf.h"
#include "dsd_decoder.h"

namespace DSDcc
{

const int DSDYSF::m_fichInterleave[100] = {
        0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95,
        1, 6, 11, 16, 21, 26, 31, 36, 41, 46, 51, 56, 61, 66, 71, 76, 81, 86, 91, 96,
        2, 7, 12, 17, 22, 27, 32, 37, 42, 47, 52, 57, 62, 67, 72, 77, 82, 87, 92, 97,
        3, 8, 13, 18, 23, 28, 33, 38, 43, 48, 53, 58, 63, 68, 73, 78, 83, 88, 93, 98,
        4, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 59, 64, 69, 74, 79, 84, 89, 94, 99,
};

const int DSDYSF::m_dchInterleave[180] = {
        0,   9,  18,  27,  36,  45,  54,  63,  72,  81,  90,  99, 108, 117, 126, 135, 144, 153, 162, 171,
        1,  10,  19,  28,  37,  46,  55,  64,  73,  82,  91, 100, 109, 118, 127, 136, 145, 154, 163, 172,
        2,  11,  20,  29,  38,  47,  56,  65,  74,  83,  92, 101, 110, 119, 128, 137, 146, 155, 164, 173,
        3,  12,  21,  30,  39,  48,  57,  66,  75,  84,  93, 102, 111, 120, 129, 138, 147, 156, 165, 174,
        4,  13,  22,  31,  40,  49,  58,  67,  76,  85,  94, 103, 112, 121, 130, 139, 148, 157, 166, 175,
        5,  14,  23,  32,  41,  50,  59,  68,  77,  86,  95, 104, 113, 122, 131, 140, 149, 158, 167, 176,
        6,  15,  24,  33,  42,  51,  60,  69,  78,  87,  96, 105, 114, 123, 132, 141, 150, 159, 168, 177,
        7,  16,  25,  34,  43,  52,  61,  70,  79,  88,  97, 106, 115, 124, 133, 142, 151, 160, 169, 178,
        8,  17,  26,  35,  44,  53,  62,  71,  80,  89,  98, 107, 116, 125, 134, 143, 152, 161, 170, 179
};

/*
 * DMR AMBE interleave schedule
 */
// bit 1
const int DSDYSF::rW[36] = {
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 2,
  0, 2, 0, 2, 0, 2,
  0, 2, 0, 2, 0, 2
};

const int DSDYSF::rX[36] = {
  23, 10, 22, 9, 21, 8,
  20, 7, 19, 6, 18, 5,
  17, 4, 16, 3, 15, 2,
  14, 1, 13, 0, 12, 10,
  11, 9, 10, 8, 9, 7,
  8, 6, 7, 5, 6, 4
};

// bit 0
const int DSDYSF::rY[36] = {
  0, 2, 0, 2, 0, 2,
  0, 2, 0, 3, 0, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3,
  1, 3, 1, 3, 1, 3
};

const int DSDYSF::rZ[36] = {
  5, 3, 4, 2, 3, 1,
  2, 0, 1, 13, 0, 12,
  22, 11, 21, 10, 20, 9,
  19, 8, 18, 7, 17, 6,
  16, 5, 15, 4, 14, 3,
  13, 2, 12, 1, 11, 0
};

DSDYSF::DSDYSF(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbolIndex(0),
        m_viterbiFICH(2, Viterbi::Poly25y, true),
        m_crc(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff),
        m_pn(0x1c9),
		m_fichError(FICHNoError)
{
    w = 0;
    x = 0;
    y = 0;
    z = 0;
}

DSDYSF::~DSDYSF()
{
}

void DSDYSF::init()
{
    m_symbolIndex = 0;
}

void DSDYSF::process() // just pass the frames for now
{
    unsigned char dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get current dibit

    if (m_symbolIndex < 100)
    {
        processFICH(m_symbolIndex, dibit);
    }
    else if (m_symbolIndex < 480 - 20) // frame is 480 dibits and sync is 20 dibits
    {
        switch (m_fich.getFrameInformation())
        {
        case FIHeader:
        case FITerminator:
            processHeader(m_symbolIndex - 100, dibit);
            break;
        case FICommunication:
            {
                switch (m_fich.getDataType())
                {
                case DTVoiceData1:
                    processVD1(m_symbolIndex - 100, dibit);
                    break;
                case DTVoiceData2:
                    processVD2(m_symbolIndex - 100, dibit);
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            break;
        }
    }
    else
    {
        m_dsdDecoder->resetFrameSync(); // end
        return;
    }

    m_symbolIndex++;
}

void DSDYSF::processFICH(int symbolIndex, unsigned char dibit)
{
    m_fichRaw[m_fichInterleave[symbolIndex]] = dibit;

    if (symbolIndex == 100-1)
    {
        m_viterbiFICH.decodeFromSymbols(m_fichGolay, m_fichRaw, 100, 0);
        int i = 0;

        for (; i < 4; i++)
        {
            if (m_golay_24_12.decode(&m_fichGolay[24*i]))
            {
                memcpy(&m_fichBits[12*i], &m_fichGolay[24*i], 12);
            }
            else
            {
                std::cerr << "DSDYSF::processFICH: Golay KO #" << i << std::endl;
                m_fichError = FICHErrorGolay;
                break;
            }
        }

        if (i == 4) // decoding OK
        {
            unsigned char bytes[6];

            if (checkCRC16(m_fichBits, 4))
            {
                memcpy(&m_fich, m_fichBits, 32);
//                std::cerr << "DSDYSF::processFICH: CRC OK: " << m_fich << std::endl;
                m_fichError = FICHNoError;
            }
            else
            {
                std::cerr << "DSDYSF::processFICH: CRC KO" << std::endl;
                m_fichError = FICHErrorCRC;
            }
        }
    }
}

void DSDYSF::processHeader(int symbolIndex, unsigned char dibit)
{
    if (symbolIndex < 36)         // DCH1(0)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex]] = dibit;
    }
    else if (symbolIndex < 2*36)  // DCH2(0)
    {
        m_dch2Raw[m_dchInterleave[symbolIndex - 36]] = dibit;
    }
    else if (symbolIndex < 3*36)  // DCH1(1)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex - 36]] = dibit;
    }
    else if (symbolIndex < 4*36)  // DCH2(1)
    {
        m_dch2Raw[m_dchInterleave[symbolIndex - 2*36]] = dibit;
    }
    else if (symbolIndex < 5*36)  // DCH1(2)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex - 2*36]] = dibit;
    }
    else if (symbolIndex < 6*36)  // DCH2(2)
    {
        m_dch2Raw[m_dchInterleave[symbolIndex - 3*36]] = dibit;
    }
    else if (symbolIndex < 7*36)  // DCH1(3)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex - 3*36]] = dibit;
    }
    else if (symbolIndex < 8*36)  // DCH2(3)
    {
        m_dch2Raw[m_dchInterleave[symbolIndex - 4*36]] = dibit;
    }
    else if (symbolIndex < 9*36)  // DCH1(4)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex - 4*36]] = dibit;
    }
    else if (symbolIndex < 10*36) // DCH2(4)
    {
        m_dch2Raw[m_dchInterleave[symbolIndex - 5*36]] = dibit;
    }

    if (symbolIndex == 360 - 1) // final
    {
        unsigned char bytes[22];

        m_viterbiFICH.decodeFromSymbols(m_dch1Bits, m_dch1Raw, 180, 0);
        m_viterbiFICH.decodeFromSymbols(m_dch2Bits, m_dch2Raw, 180, 0);

        if (checkCRC16(m_dch1Bits, 20, bytes)) // CSD1
        {
            processCSD1(bytes);
        }
        else
        {
            std::cerr << "DSDYSF::processHeader: DCH1 CRC KO" << std::endl;
        }

        if (checkCRC16(m_dch2Bits, 20, bytes)) // CSD2
        {
            processCSD2(bytes);
        }
        else
        {
            std::cerr << "DSDYSF::processHeader: DCH2 CRC KO" << std::endl;
        }
    }
}

void DSDYSF::processCSD1(unsigned char *dchBytes)
{
    if (m_fich.getCallMode() == CMRadioID)
    {
        memcpy(m_destId, dchBytes, 5);
        m_destId[5] = '\0';
        memcpy(m_srcId, &dchBytes[5], 5);
        m_destId[5] = '\0';
    }
    else
    {
        memcpy(m_dest, dchBytes, 10);
        m_dest[10] = '\0';
        memcpy(m_src, &dchBytes[10], 10);
        m_src[10] = '\0';
//        std::cerr << "DSDYSF::processCSD1: Dest: " << m_dest << " Src: " << m_src << std::endl;
    }
}

void DSDYSF::processCSD2(unsigned char *dchBytes)
{
    memcpy(m_downlink, dchBytes, 10);
    m_downlink[10] = '\0';
    memcpy(m_uplink, &dchBytes[10], 10);
    m_uplink[10] = '\0';
//    std::cerr << "DSDYSF::processCSD2:  D/L: " << m_downlink << " U/L: " << m_uplink << std::endl;
}

void DSDYSF::processCSD3_1(unsigned char *dchBytes)
{
    memcpy(m_rem1, dchBytes, 5);
    m_rem1[6] = '\0';
    memcpy(m_rem2, &dchBytes[5], 5);
    m_rem2[6] = '\0';
    std::cerr << "DSDYSF::processCSD3_1: Rem1: " << m_rem1 << std::endl;
    std::cerr << "DSDYSF::processCSD3_1: Rem2: " << m_rem2 << std::endl;
}

void DSDYSF::processCSD3_2(unsigned char *dchBytes)
{
    memcpy(m_rem3, dchBytes, 5);
    m_rem3[6] = '\0';
    memcpy(m_rem4, &dchBytes[5], 5);
    m_rem4[6] = '\0';
    std::cerr << "DSDYSF::processCSD3_2: Rem3: " << m_rem3 << std::endl;
    std::cerr << "DSDYSF::processCSD3_2: Rem4: " << m_rem4 << std::endl;
}

void DSDYSF::processVD1(int symbolIndex, unsigned char dibit)
{
    if (symbolIndex < 36)         // DCH(0)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex]] = dibit;
    }
    else if (symbolIndex < 2*36)  // VCH(0)
    {
        processMBE(symbolIndex - 36, dibit);
    }
    else if (symbolIndex < 3*36)  // DCH(1)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex - 36]] = dibit;
    }
    else if (symbolIndex < 4*36)  // VCH(1)
    {
        processMBE(symbolIndex - 3*36, dibit);
    }
    else if (symbolIndex < 5*36)  // DCH(2)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex - 2*36]] = dibit;
    }
    else if (symbolIndex < 6*36)  // VCH(2)
    {
        processMBE(symbolIndex - 5*36, dibit);
    }
    else if (symbolIndex < 7*36)  // DCH(3)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex - 3*36]] = dibit;
    }
    else if (symbolIndex < 8*36)  // VCH(3)
    {
        processMBE(symbolIndex - 7*36, dibit);
    }
    else if (symbolIndex < 9*36)  // DCH(4)
    {
        m_dch1Raw[m_dchInterleave[symbolIndex - 4*36]] = dibit;

        if (symbolIndex == 9*36 - 1)
        {
            unsigned char bytes[22];

            m_viterbiFICH.decodeFromSymbols(m_dch1Bits, m_dch1Raw, 180, 0);

            if (checkCRC16(m_dch1Bits, 20, bytes)) // CSD
            {
                switch (m_fich.getFrameNumber())
                {
                case 0: // CSD1
                    processCSD1(bytes);
                    break;
                case 1: // CSD2
                    processCSD2(bytes);
                    break;
                case 2: // CSD3
                    processCSD3_1(bytes);
                    processCSD3_2(&bytes[10]);
                    break;
                default:
                    break;
                }
            }
        }
    }
    else if (symbolIndex < 10*36) // VCH(4)
    {
        processMBE(symbolIndex - 9*36, dibit);
    }
}

void DSDYSF::processVD2(int symbolIndex, unsigned char dibit)
{
    if (symbolIndex < 20) // DCH(0) - reuse FICH buffer
    {
        m_fichRaw[m_fichInterleave[symbolIndex]] = dibit;
    }
    else if (symbolIndex < 20 + 52) // VCH(0) and VeCH(0)
    {
        processVD2Voice(symbolIndex - 20, dibit);
    }
    else if (symbolIndex < 2*20 + 52) // DCH(1)
    {
        m_fichRaw[m_fichInterleave[symbolIndex - 52]] = dibit;
    }
    else if (symbolIndex < 2*20 + 2*52) // VCH(1) and VeCH(1)
    {
        processVD2Voice(symbolIndex - (2*20 + 52), dibit);
    }
    else if (symbolIndex < 3*20 + 2*52) // DCH(2)
    {
        m_fichRaw[m_fichInterleave[symbolIndex - 2*52]] = dibit;
    }
    else if (symbolIndex < 3*20 + 3*52) // VCH(2) and VeCH(2)
    {
        processVD2Voice(symbolIndex - (3*20 + 2*52), dibit);
    }
    else if (symbolIndex < 4*20 + 3*52) // DCH(3)
    {
        m_fichRaw[m_fichInterleave[symbolIndex - 3*52]] = dibit;
    }
    else if (symbolIndex < 4*20 + 4*52) // VCH(3) and VeCH(3)
    {
        processVD2Voice(symbolIndex - (4*20 + 3*52), dibit);
    }
    else if (symbolIndex < 5*20 + 4*52) // DCH(4)
    {
        m_fichRaw[m_fichInterleave[symbolIndex - 4*52]] = dibit;

        if (symbolIndex == (5*20 + 4*52) - 1) // Final DCH
        {
            unsigned char bytes[12];

            m_viterbiFICH.decodeFromSymbols(m_fichGolay, m_fichRaw, 100, 0); // reuse FICH

            if (checkCRC16(m_fichGolay, 10, bytes))
            {
                switch (m_fich.getFrameNumber())
                {
                case 0:
                    memcpy(m_dest, bytes, 10);
                    m_dest[10] = '\0';
//                    std::cerr << "DSDYSF::processVD2: Dest: " << m_dest << std::endl;
                    break;
                case 1:
                    memcpy(m_src, bytes, 10);
                    m_src[10] = '\0';
//                    std::cerr << "DSDYSF::processVD2:  Src: " << m_src << std::endl;
                    break;
                case 2:
                    memcpy(m_downlink, bytes, 10);
                    m_downlink[10] = '\0';
//                    std::cerr << "DSDYSF::processVD2:  D/L: " << m_downlink << std::endl;
                    break;
                case 3:
                    memcpy(m_uplink, bytes, 10);
                    m_uplink[10] = '\0';
//                    std::cerr << "DSDYSF::processVD2:  U/L: " << m_uplink << std::endl;
                    break;
                case 4:
                    processCSD3_1(bytes);
                    break;
                case 5:
                    processCSD3_2(bytes);
                    break;
                default:
                    break;
                }
            }
        }
    }
    else if (symbolIndex < 5*20 + 5*52) // DCH(4)
    {
        processVD2Voice(symbolIndex - (5*20 + 4*52), dibit);
    }
}

void DSDYSF::processVD2Voice(int mbeIndex, unsigned char dibit)
{
// TODO
}

void DSDYSF::processMBE(int mbeIndex, unsigned char dibit)
{
	if (mbeIndex == 0) // init
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

	storeSymbolDV(m_dsdDecoder->m_mbeDVFrame1, mbeIndex, dibit); // store dibit for DVSI hardware decoder

	if (mbeIndex == 36-1) // finalize
	{
        m_dsdDecoder->m_mbeDecoder1.processFrame(0, m_dsdDecoder->ambe_fr, 0);
        m_dsdDecoder->m_mbeDVReady1 = true; // Indicate that a DVSI frame is available
	}
}

void DSDYSF::storeSymbolDV(unsigned char *mbeFrame, int dibitindex, unsigned char dibit, bool invertDibit)
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

bool DSDYSF::checkCRC16(unsigned char *bits,  unsigned long nbBytes, unsigned char *xoredBytes)
{
    unsigned char bytes[22];
//    std::cerr << "DSDYSF::checkCRC16: value: ";

    for (int i = 0; i < nbBytes+2; i++)
    {
        bytes[i] = (bits[8*i+0]<<7)
                + (bits[8*i+1]<<6)
                + (bits[8*i+2]<<5)
                + (bits[8*i+3]<<4)
                + (bits[8*i+4]<<3)
                + (bits[8*i+5]<<2)
                + (bits[8*i+6]<<1)
                + (bits[8*i+7]<<0);

        if (xoredBytes)
        {
            xoredBytes[i] = bytes[i] ^ m_pn.getByte(i);
        }
//        std::cerr << std::hex << (int) bytes[i] << " ";
    }

    unsigned int crc = (bytes[nbBytes]<<8) + bytes[nbBytes+1];

//    std::cerr << "crc: " << std::hex << crc << std::endl;

    return m_crc.crctablefast(bytes, nbBytes) == crc;
}


} // namespace DSDcc
