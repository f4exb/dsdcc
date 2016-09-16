///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// Class specialized for K=3 decoding                                            //
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

#include "viterbi3.h"

namespace DSDcc
{

Viterbi3::Viterbi3(int n, const unsigned int *polys, bool msbFirst) :
        Viterbi(3, n, polys, msbFirst)
{
}

Viterbi3::~Viterbi3()
{
}

void Viterbi3::decodeFromSymbols(
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

        m_traceback = new unsigned char[4 * nbSymbols];
        m_pathMetrics = new uint32_t[4];
        m_nbSymbolsMax = nbSymbols;
    }

    // initial path metrics state
    memset(m_pathMetrics, Viterbi::m_maxMetric, sizeof(uint32_t) * (1<<(m_k-1)));
    m_pathMetrics[startstate] = 0;

    for (int is = 0; is < nbSymbols; is++)
    {
//        std::cerr << "Viterbi3::decodeFromSymbols: S[" << is << "]=" << (int) symbols[is] << std::endl;

        // compute metrics
        doMetrics(
                is,
                m_branchCodes,
                symbols[is],
                m_traceback,
                &m_traceback[nbSymbols],
                &m_traceback[2*nbSymbols],
                &m_traceback[3*nbSymbols],
                m_pathMetrics
        );
    } // symbols

    // trace back

    uint32_t minPathMetric;
    unsigned int minPathIndex;

    for (int i = 0; i < 4; i++)
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
            m_traceback,
            &m_traceback[nbSymbols],
            &m_traceback[2*nbSymbols],
            &m_traceback[3*nbSymbols]
    );
}

void Viterbi3::doMetrics(
        int n,
        unsigned char *branchCodes,
        unsigned char symbol,
        unsigned char *m_pathMemory0,
        unsigned char *m_pathMemory1,
        unsigned char *m_pathMemory2,
        unsigned char *m_pathMemory3,
        uint32_t  *m_pathMetric
)
{
    uint32_t tempMetric[4];
    uint32_t metric[8];
    int loop;

    uint32_t m1;
    uint32_t m2;

    // Treillis edges:

                                                   // State Received
    metric[0] = NbOnes[branchCodes[0] ^ symbol]; // 00    0
    metric[1] = NbOnes[branchCodes[1] ^ symbol]; // 00    1
    metric[2] = NbOnes[branchCodes[2] ^ symbol]; // 01    0
    metric[3] = NbOnes[branchCodes[3] ^ symbol]; // 01    1
    metric[4] = NbOnes[branchCodes[4] ^ symbol]; // 10    0
    metric[5] = NbOnes[branchCodes[5] ^ symbol]; // 10    1
    metric[6] = NbOnes[branchCodes[6] ^ symbol]; // 11    0
    metric[7] = NbOnes[branchCodes[7] ^ symbol]; // 11    1

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

    // Pres. state = S2, Prev. state = S0 & S1
    m1 = metric[1] + m_pathMetric[0];
    m2 = metric[3] + m_pathMetric[1];

    if (m1 < m2)
    {
        m_pathMemory2[n] = 0; // upper path (S0)
        tempMetric[2] = m1;
    }
    else
    {
        m_pathMemory2[n] = 1; // lower path (S1)
        tempMetric[2] = m2;
    }

    // Pres. state = S3, Prev. state = S2 & S3
    m1 = metric[5] + m_pathMetric[2];
    m2 = metric[7] + m_pathMetric[3];

    if (m1 < m2)
    {
        m_pathMemory3[n] = 2; // upper path (S2)
        tempMetric[3] = m1;
    }
    else
    {
        m_pathMemory3[n] = 3; // lower path (S3)
        tempMetric[3] = m2;
    }; // end else - if

    // Store new path metrics

    m_pathMetric[0] = tempMetric[0];
    m_pathMetric[1] = tempMetric[1];
    m_pathMetric[2] = tempMetric[2];
    m_pathMetric[3] = tempMetric[3];

} // end function ViterbiDecode


void Viterbi3::traceBack (
        int nbSymbols,
        unsigned int startState,
        unsigned char *out,
        unsigned char *m_pathMemory0,
        unsigned char *m_pathMemory1,
        unsigned char *m_pathMemory2,
        unsigned char *m_pathMemory3
)
{
    unsigned int state = startState;

    for (int loop = nbSymbols - 1; loop >= 0; loop--)
    {
        // TODO: store directly the integer state value in path memory
        switch (state)
        {
        case 0: // if state S0, Prev. state = S0 | S1
            state = m_pathMemory0[loop];
            out[loop] = 0;
            break;

        case 1: // if state S1, Prev. state = S2 | S3
            state = m_pathMemory1[loop];
            out[loop] = 0;
            break;

        case 2: // if state S2, Prev. state = S0 | S1
            state = m_pathMemory2[loop];
            out[loop] = 1;
            break;

        case 3: // if state S3, Prev. state = S2 | S3
            state = m_pathMemory3[loop];
            out[loop] = 1;
            break;

        }; // end switch
    }; // end for
}

}


