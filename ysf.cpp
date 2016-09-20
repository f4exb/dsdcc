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

DSDYSF::DSDYSF(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbolIndex(0),
        m_viterbiFICH(2, Viterbi::Poly25y, true),
        m_crc(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff),
		m_fichError(FICHNoError)
{
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

bool DSDYSF::checkCRC16(unsigned char *bits, unsigned long nbBytes)
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
//        std::cerr << std::hex << (int) bytes[i] << " ";
    }

    unsigned int crc = (bytes[nbBytes]<<8) + bytes[nbBytes+1];

//    std::cerr << "crc: " << std::hex << crc << std::endl;

    return m_crc.crctablefast(bytes, nbBytes) == crc;
}


} // namespace DSDcc
