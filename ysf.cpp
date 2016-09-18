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

//const int DSDYSF::m_fichInterleave[100] = {
//        0,  20, 40, 60, 80,
//        1,  21, 41, 61, 81,
//        2,  22, 42, 62, 82,
//        3,  23, 43, 63, 83,
//        4,  24, 44, 64, 84,
//        5,  25, 45, 65, 85,
//        6,  26, 46, 66, 86,
//        7,  27, 47, 67, 87,
//        8,  28, 48, 68, 88,
//        9,  29, 49, 69, 89,
//        10, 30, 50, 70, 90,
//        11, 31, 51, 71, 91,
//        12, 32, 52, 72, 92,
//        13, 33, 53, 73, 93,
//        14, 34, 54, 74, 94,
//        15, 35, 55, 75, 95,
//        16, 36, 56, 76, 96,
//        17, 37, 57, 77, 97,
//        18, 38, 58, 78, 98,
//        19, 39, 59, 79, 99,
//};

DSDYSF::DSDYSF(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbolIndex(0),
        m_viterbiFICH(2, Viterbi::Poly25y, true)
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
                memcpy(&m_fich[12*i], &m_fichGolay[24*i], 12);
            }
            else
            {
                std::cerr << "DSDYSF::processFICH: Golay KO #" << i << std::endl;
                break;
            }
        }

        if (i == 4) // decoding OK
        {
            std::cerr << "DSDYSF::processFICH: Golay OK" << std::endl;

            if (checkCRC16(m_fich, 32))
            {
                std::cerr << "DSDYSF::processFICH: CRC OK" << std::endl;
            }
            else
            {
                std::cerr << "DSDYSF::processFICH: CRC KO" << std::endl;
            }
        }
    }
}

bool DSDYSF::checkCRC16(unsigned char *bits, int nbBits)
{
    memcpy(m_bitWork, bits, nbBits);
    memset(&m_bitWork[nbBits], 0, 16);

    for (int i = 0; i < nbBits; i++)
    {
        if (m_bitWork[i] == 1) // divide by X^7+X^3+1 (10001001)
        {
            m_bitWork[i]     = 0; // X^16 16-16 = +0
            m_bitWork[i+4]  ^= 1; // X^12 16-12 = +4
            m_bitWork[i+11] ^= 1; // X^5  16-5  = +11
            m_bitWork[i+16] ^= 1; // 1    16-0  = +16
        }
    }

    if (memcmp(&bits[nbBits], &m_bitWork[nbBits], 16) == 0) // CRC OK
    {
        return true;
    }
    else
    {
        return false;
    }
}


} // namespace DSDcc
