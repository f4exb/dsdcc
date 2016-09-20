///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// Class specialized for K=5 decoding                                            //
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
#include <limits.h>

#include "viterbi5.h"

namespace DSDcc
{

Viterbi5::Viterbi5(int n, const unsigned int *polys, bool msbFirst) :
        Viterbi(5, n, polys, msbFirst)
{
}

Viterbi5::~Viterbi5()
{
}

void Viterbi5::decodeFromBits(
        unsigned char *dataBits,      //!< Decoded output data bits
        const unsigned char *bits,    //!< Input bits
        unsigned int nbBits,          //!< Number of imput bits
        unsigned int startstate)      //!< Encoder starting state

{
    if (nbBits > m_nbBitsMax)
    {
        if (m_symbols) {
            delete[] m_symbols;
        }

        m_symbols = new unsigned char[nbBits/m_n];
        m_nbBitsMax = nbBits;
    }

    for (int i = 0; i < nbBits; i += m_n)
    {
        m_symbols[i/m_n] = bits[i];

        for (int j = m_n-1; j > 0; j--)
        {
            m_symbols[i/m_n] += bits[i+j] << j;
        }
    }

    decodeFromSymbols(dataBits, m_symbols, nbBits/m_n, startstate);
}

void Viterbi5::decodeFromSymbols(
        unsigned char *dataBits,      //!< Decoded output data bits
        const unsigned char *symbols, //!< Input symbols
        unsigned int nbSymbols,       //!< Number of imput symbols
        unsigned int startstate)      //!< Encoder starting state

{
    if (nbSymbols > m_nbSymbolsMax)
    {
        if (m_traceback) {
            delete[] m_traceback;
        }

        if (m_pathMetrics) {
            delete[] m_pathMetrics;
        }

        m_traceback = new unsigned char[16 * nbSymbols];
        m_pathMetrics = new uint32_t[16];
        m_nbSymbolsMax = nbSymbols;
    }

    // initial path metrics state
    memset(m_pathMetrics, Viterbi::m_maxMetric, 16 * sizeof(uint32_t));
    m_pathMetrics[startstate] = 0;

    for (int is = 0; is < nbSymbols; is++)
    {
//        std::cerr << "Viterbi3::decodeFromSymbols: S[" << is << "]=" << (int) symbols[is] << std::endl;

        // compute metrics
        doMetrics(
                is,
                m_branchCodes,
                symbols[is],
                &m_traceback[0*nbSymbols],
                &m_traceback[1*nbSymbols],
                &m_traceback[2*nbSymbols],
                &m_traceback[3*nbSymbols],
                &m_traceback[4*nbSymbols],
                &m_traceback[5*nbSymbols],
                &m_traceback[6*nbSymbols],
                &m_traceback[7*nbSymbols],
                &m_traceback[8*nbSymbols],
                &m_traceback[9*nbSymbols],
                &m_traceback[10*nbSymbols],
                &m_traceback[11*nbSymbols],
                &m_traceback[12*nbSymbols],
                &m_traceback[13*nbSymbols],
                &m_traceback[14*nbSymbols],
                &m_traceback[15*nbSymbols],
                m_pathMetrics
        );
    } // symbols

    // trace back

    uint32_t minPathMetric = m_pathMetrics[0];
    unsigned int minPathIndex = 0;

    for (int i = 1; i < 16; i++)
    {
        if (m_pathMetrics[i] < minPathMetric)
        {
            minPathMetric = m_pathMetrics[i];
            minPathIndex = i;
        }
    }

//    std::cerr << "Viterbi3::decodeFromSymbols: last path node: " << minPathIndex << std::endl;

    traceBack(
            nbSymbols,
            minPathIndex,
            dataBits,
            &m_traceback[0*nbSymbols],
            &m_traceback[1*nbSymbols],
            &m_traceback[2*nbSymbols],
            &m_traceback[3*nbSymbols],
            &m_traceback[4*nbSymbols],
            &m_traceback[5*nbSymbols],
            &m_traceback[6*nbSymbols],
            &m_traceback[7*nbSymbols],
            &m_traceback[8*nbSymbols],
            &m_traceback[9*nbSymbols],
            &m_traceback[10*nbSymbols],
            &m_traceback[11*nbSymbols],
            &m_traceback[12*nbSymbols],
            &m_traceback[13*nbSymbols],
            &m_traceback[14*nbSymbols],
            &m_traceback[15*nbSymbols]
    );
}

void Viterbi5::doMetrics(
        int n,
        unsigned char *branchCodes,
        unsigned char symbol,
        unsigned char *m_pathMemory0,
        unsigned char *m_pathMemory1,
        unsigned char *m_pathMemory2,
        unsigned char *m_pathMemory3,
        unsigned char *m_pathMemory4,
        unsigned char *m_pathMemory5,
        unsigned char *m_pathMemory6,
        unsigned char *m_pathMemory7,
        unsigned char *m_pathMemory8,
        unsigned char *m_pathMemory9,
        unsigned char *m_pathMemory10,
        unsigned char *m_pathMemory11,
        unsigned char *m_pathMemory12,
        unsigned char *m_pathMemory13,
        unsigned char *m_pathMemory14,
        unsigned char *m_pathMemory15,
        uint32_t  *m_pathMetric
)
{
    uint32_t tempMetric[16];
    uint32_t metric[32];

    uint32_t m1;
    uint32_t m2;

    // Treillis edges:
                                                   // State Received
    metric[0]  = NbOnes[branchCodes[0]  ^ symbol]; // 0000    0
    metric[1]  = NbOnes[branchCodes[1]  ^ symbol]; // 0000    1
    metric[2]  = NbOnes[branchCodes[2]  ^ symbol]; // 0001    0
    metric[3]  = NbOnes[branchCodes[3]  ^ symbol]; // 0001    1
    metric[4]  = NbOnes[branchCodes[4]  ^ symbol]; // 0010    0
    metric[5]  = NbOnes[branchCodes[5]  ^ symbol]; // 0010    1
    metric[6]  = NbOnes[branchCodes[6]  ^ symbol]; // 0011    0
    metric[7]  = NbOnes[branchCodes[7]  ^ symbol]; // 0011    1
    metric[8]  = NbOnes[branchCodes[8]  ^ symbol]; // 0100    0
    metric[9]  = NbOnes[branchCodes[9]  ^ symbol]; // 0100    1
    metric[10] = NbOnes[branchCodes[10] ^ symbol]; // 0101    0
    metric[11] = NbOnes[branchCodes[11] ^ symbol]; // 0101    1
    metric[12] = NbOnes[branchCodes[12] ^ symbol]; // 0110    0
    metric[13] = NbOnes[branchCodes[13] ^ symbol]; // 0110    1
    metric[14] = NbOnes[branchCodes[14] ^ symbol]; // 0111    0
    metric[15] = NbOnes[branchCodes[15] ^ symbol]; // 0111    1
    metric[16] = NbOnes[branchCodes[16] ^ symbol]; // 1000    0
    metric[17] = NbOnes[branchCodes[17] ^ symbol]; // 1000    1
    metric[18] = NbOnes[branchCodes[18] ^ symbol]; // 1001    0
    metric[19] = NbOnes[branchCodes[19] ^ symbol]; // 1001    1
    metric[20] = NbOnes[branchCodes[20] ^ symbol]; // 1010    0
    metric[21] = NbOnes[branchCodes[21] ^ symbol]; // 1010    1
    metric[22] = NbOnes[branchCodes[22] ^ symbol]; // 1011    0
    metric[23] = NbOnes[branchCodes[23] ^ symbol]; // 1011    1
    metric[24] = NbOnes[branchCodes[24] ^ symbol]; // 1100    0
    metric[25] = NbOnes[branchCodes[25] ^ symbol]; // 1100    1
    metric[26] = NbOnes[branchCodes[26] ^ symbol]; // 1101    0
    metric[27] = NbOnes[branchCodes[27] ^ symbol]; // 1101    1
    metric[28] = NbOnes[branchCodes[28] ^ symbol]; // 1110    0
    metric[29] = NbOnes[branchCodes[29] ^ symbol]; // 1110    1
    metric[30] = NbOnes[branchCodes[30] ^ symbol]; // 1111    0
    metric[31] = NbOnes[branchCodes[31] ^ symbol]; // 1111    1

    // This hardcodes the Treillis structure:

    // Pres. state = S0, Prev. state = S0 & S1
    m1 = metric[0] + m_pathMetric[0];
    m2 = metric[2] + m_pathMetric[1];

    if (m1 < m2)
    {
        m_pathMemory0[n] = 0; // upper path (S0)
        tempMetric[0] = m1;
    }
    else
    {
        m_pathMemory0[n] = 1; // lower path (S1)
        tempMetric[0] = m2;
    }; // end else - if

    // Pres. state = S1, Prev. state = S2 & S3
    m1 = metric[4] + m_pathMetric[2];
    m2 = metric[6] + m_pathMetric[3];

    if (m1 < m2)
    {
        m_pathMemory1[n] = 2; // upper path (S2)
        tempMetric[1] = m1;
    }
    else
    {
        m_pathMemory1[n] = 3; // lower path (S3)
        tempMetric[1] = m2;
    }; // end else - if

    // Pres. state = S2, Prev. state = S4 & S5
    m1 = metric[8]  + m_pathMetric[4];
    m2 = metric[10] + m_pathMetric[5];

    if (m1 < m2)
    {
        m_pathMemory2[n] = 4; // upper path (S4)
        tempMetric[2] = m1;
    }
    else
    {
        m_pathMemory2[n] = 5; // lower path (S5)
        tempMetric[2] = m2;
    }

    // Pres. state = S3, Prev. state = S6 & S7
    m1 = metric[12] + m_pathMetric[6];
    m2 = metric[14] + m_pathMetric[7];

    if (m1 < m2)
    {
        m_pathMemory3[n] = 6; // upper path (S6)
        tempMetric[3] = m1;
    }
    else
    {
        m_pathMemory3[n] = 7; // lower path (S7)
        tempMetric[3] = m2;
    }; // end else - if

    // Pres. state = S4, Prev. state = S8 & S9
    m1 = metric[16] + m_pathMetric[8];
    m2 = metric[18] + m_pathMetric[9];

    if (m1 < m2)
    {
        m_pathMemory4[n] = 8; // upper path (S8)
        tempMetric[4] = m1;
    }
    else
    {
        m_pathMemory4[n] = 9; // lower path (S9)
        tempMetric[4] = m2;
    }; // end else - if

    // Pres. state = S5, Prev. state = S10 & S11
    m1 = metric[20] + m_pathMetric[10];
    m2 = metric[22] + m_pathMetric[11];

    if (m1 < m2)
    {
        m_pathMemory5[n] = 10; // upper path (S10)
        tempMetric[5] = m1;
    }
    else
    {
        m_pathMemory5[n] = 11; // lower path (S11)
        tempMetric[5] = m2;
    }; // end else - if

    // Pres. state = S6, Prev. state = S12 & S13
    m1 = metric[24] + m_pathMetric[12];
    m2 = metric[26] + m_pathMetric[13];

    if (m1 < m2)
    {
        m_pathMemory6[n] = 12; // upper path (S12)
        tempMetric[6] = m1;
    }
    else
    {
        m_pathMemory6[n] = 13; // lower path (S13)
        tempMetric[6] = m2;
    }; // end else - if

    // Pres. state = S7, Prev. state = S14 & S15
    m1 = metric[28] + m_pathMetric[14];
    m2 = metric[30] + m_pathMetric[15];

    if (m1 < m2)
    {
        m_pathMemory7[n] = 14; // upper path (S14)
        tempMetric[7] = m1;
    }
    else
    {
        m_pathMemory7[n] = 15; // lower path (S15)
        tempMetric[7] = m2;
    }; // end else - if

    // -- ones --

    // Pres. state = S8, Prev. state = S0 & S1
    m1 = metric[1] + m_pathMetric[0];
    m2 = metric[3] + m_pathMetric[1];

    if (m1 < m2)
    {
        m_pathMemory8[n] = 0; // upper path (S0)
        tempMetric[8] = m1;
    }
    else
    {
        m_pathMemory8[n] = 1; // lower path (S1)
        tempMetric[8] = m2;
    }; // end else - if

    // Pres. state = S9, Prev. state = S2 & S3
    m1 = metric[5] + m_pathMetric[2];
    m2 = metric[7] + m_pathMetric[3];

    if (m1 < m2)
    {
        m_pathMemory9[n] = 2; // upper path (S2)
        tempMetric[9] = m1;
    }
    else
    {
        m_pathMemory9[n] = 3; // lower path (S3)
        tempMetric[9] = m2;
    }; // end else - if

    // Pres. state = S10, Prev. state = S4 & S5
    m1 = metric[9]  + m_pathMetric[4];
    m2 = metric[11] + m_pathMetric[5];

    if (m1 < m2)
    {
        m_pathMemory10[n] = 4; // upper path (S4)
        tempMetric[10] = m1;
    }
    else
    {
        m_pathMemory10[n] = 5; // lower path (S5)
        tempMetric[10] = m2;
    }

    // Pres. state = S11, Prev. state = S6 & S7
    m1 = metric[13] + m_pathMetric[6];
    m2 = metric[15] + m_pathMetric[7];

    if (m1 < m2)
    {
        m_pathMemory11[n] = 6; // upper path (S6)
        tempMetric[11] = m1;
    }
    else
    {
        m_pathMemory11[n] = 7; // lower path (S7)
        tempMetric[11] = m2;
    }; // end else - if

    // Pres. state = S12, Prev. state = S8 & S9
    m1 = metric[17] + m_pathMetric[8];
    m2 = metric[19] + m_pathMetric[9];

    if (m1 < m2)
    {
        m_pathMemory12[n] = 8; // upper path (S8)
        tempMetric[12] = m1;
    }
    else
    {
        m_pathMemory12[n] = 9; // lower path (S9)
        tempMetric[12] = m2;
    }; // end else - if

    // Pres. state = S13, Prev. state = S10 & S11
    m1 = metric[21] + m_pathMetric[10];
    m2 = metric[23] + m_pathMetric[11];

    if (m1 < m2)
    {
        m_pathMemory13[n] = 10; // upper path (S10)
        tempMetric[13] = m1;
    }
    else
    {
        m_pathMemory13[n] = 11; // lower path (S11)
        tempMetric[13] = m2;
    }; // end else - if

    // Pres. state = S14, Prev. state = S12 & S13
    m1 = metric[25] + m_pathMetric[12];
    m2 = metric[27] + m_pathMetric[13];

    if (m1 < m2)
    {
        m_pathMemory14[n] = 12; // upper path (S12)
        tempMetric[14] = m1;
    }
    else
    {
        m_pathMemory14[n] = 13; // lower path (S13)
        tempMetric[14] = m2;
    }; // end else - if

    // Pres. state = S15, Prev. state = S14 & S15
    m1 = metric[29] + m_pathMetric[14];
    m2 = metric[31] + m_pathMetric[15];

    if (m1 < m2)
    {
        m_pathMemory15[n] = 14; // upper path (S14)
        tempMetric[15] = m1;
    }
    else
    {
        m_pathMemory15[n] = 15; // lower path (S15)
        tempMetric[15] = m2;
    }; // end else - if

    // Store new path metrics

    m_pathMetric[0]  = tempMetric[0];
    m_pathMetric[1]  = tempMetric[1];
    m_pathMetric[2]  = tempMetric[2];
    m_pathMetric[3]  = tempMetric[3];
    m_pathMetric[4]  = tempMetric[4];
    m_pathMetric[5]  = tempMetric[5];
    m_pathMetric[6]  = tempMetric[6];
    m_pathMetric[7]  = tempMetric[7];
    m_pathMetric[8]  = tempMetric[8];
    m_pathMetric[9]  = tempMetric[9];
    m_pathMetric[10] = tempMetric[10];
    m_pathMetric[11] = tempMetric[11];
    m_pathMetric[12] = tempMetric[12];
    m_pathMetric[13] = tempMetric[13];
    m_pathMetric[14] = tempMetric[14];
    m_pathMetric[15] = tempMetric[15];

} // end function ViterbiDecode


void Viterbi5::traceBack (
        int nbSymbols,
        unsigned int startState,
        unsigned char *out,
        unsigned char *m_pathMemory0,
        unsigned char *m_pathMemory1,
        unsigned char *m_pathMemory2,
        unsigned char *m_pathMemory3,
        unsigned char *m_pathMemory4,
        unsigned char *m_pathMemory5,
        unsigned char *m_pathMemory6,
        unsigned char *m_pathMemory7,
        unsigned char *m_pathMemory8,
        unsigned char *m_pathMemory9,
        unsigned char *m_pathMemory10,
        unsigned char *m_pathMemory11,
        unsigned char *m_pathMemory12,
        unsigned char *m_pathMemory13,
        unsigned char *m_pathMemory14,
        unsigned char *m_pathMemory15
)
{
    unsigned int state = startState;

    for (int loop = nbSymbols - 1; loop >= 0; loop--)
    {
        switch (state)
        {
        case 0: // if state S0
            state = m_pathMemory0[loop];
            out[loop] = 0;
            break;

        case 1: // if state S1
            state = m_pathMemory1[loop];
            out[loop] = 0;
            break;

        case 2: // if state S2
            state = m_pathMemory2[loop];
            out[loop] = 0;
            break;

        case 3: // if state S3
            state = m_pathMemory3[loop];
            out[loop] = 0;
            break;

        case 4: // if state S4
            state = m_pathMemory4[loop];
            out[loop] = 0;
            break;

        case 5: // if state S5
            state = m_pathMemory5[loop];
            out[loop] = 0;
            break;

        case 6: // if state S6
            state = m_pathMemory6[loop];
            out[loop] = 0;
            break;

        case 7: // if state S7
            state = m_pathMemory7[loop];
            out[loop] = 0;
            break;

        case 8: // if state S8
            state = m_pathMemory8[loop];
            out[loop] = 1;
            break;

        case 9: // if state S9
            state = m_pathMemory9[loop];
            out[loop] = 1;
            break;

        case 10: // if state S10
            state = m_pathMemory10[loop];
            out[loop] = 1;
            break;

        case 11: // if state S11
            state = m_pathMemory11[loop];
            out[loop] = 1;
            break;

        case 12: // if state S12
            state = m_pathMemory12[loop];
            out[loop] = 1;
            break;

        case 13: // if state S13
            state = m_pathMemory13[loop];
            out[loop] = 1;
            break;

        case 14: // if state S14
            state = m_pathMemory14[loop];
            out[loop] = 1;
            break;

        case 15: // if state S15
            state = m_pathMemory15[loop];
            out[loop] = 1;
            break;
        }; // end switch
    }; // end for
}

}


