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
        m_pathMetrics = new uint32_t[4 * (nbSymbols+1)];
        m_nbSymbolsMax = nbSymbols;
    }

    // initial path metrics state
    memset(m_pathMetrics, -1, sizeof(int) * (1<<(m_k-1)));
    m_pathMetrics[startstate] = 0;

    unsigned int minPathIndex;
    int minMetric;

    for (int is = 0; is < nbSymbols; is++)
    {
        minMetric = INT_MAX;
//        std::cerr << "S[" << is << "]=" << (int) symbols[is] << std::endl;

//        // compute metrics
//        doMetrics(
//            is,
//            symbols[is],
//            m_traceback,
//            &m_traceback[nbSymbols],
//            &m_traceback[2*nbSymbols],
//            &m_traceback[3*nbSymbols],
//            m_pathMetrics
//        );



        for (unsigned int ib = 0; ib < 1<<(m_k-1); ib++)
        {
            // path A

            unsigned char predA = m_predA[ib];
            unsigned int predPMIxA = is*(1<<(m_k-1)) + predA;
            unsigned char bitA = m_bitA[ib];
            unsigned char codeA = m_branchCodes[(predA<<1)+bitA];
            unsigned char bmA = NbOnes[codeA ^ symbols[is]]; // branch metric
            int pmA; // path metric

            if (m_pathMetrics[predPMIxA] < 0) // predecessor has an infinite metric
            {
                pmA = -1; // path metric is infinite
            }
            else
            {
                pmA = m_pathMetrics[predPMIxA] + bmA; // add branch metric to path metric
            }

            // path B

            unsigned char predB = m_predB[ib];
            unsigned int predPMIxB = is*(1<<(m_k-1)) + predB;
            unsigned char bitB = m_bitB[ib];
            unsigned char codeB = m_branchCodes[(predB<<1)+bitB];
            unsigned char bmB = NbOnes[codeB ^ symbols[is]]; // branch metric
            int pmB; // path metric

            if (m_pathMetrics[predPMIxB] < 0) // predecessor has an infinite metric
            {
                pmB = -1; // path metric is infinite
            }
            else
            {
                pmB = m_pathMetrics[predPMIxB] + bmB; // add branch metric to path metric
            }

            // decisions, decisions ...

//            std::cerr << "  Branch:"
//                    << " " << ib
//                    << " predA: " << (int) predA
//                    << " pm[" << (int) predA << ":" << is*(1<<(m_k-1)) << "]: " << m_pathMetrics[predPMIxA]
//                    << " bitA: " << (int) bitA
//                    << " codeA: " << (int) codeA
//                    << " bmA: " << (int) bmA
//                    << " pmA: " << pmA
//                    << " | predB: " << (int) predB
//                    << " pm[" << (int) predB << ":" << is*(1<<(m_k-1)) << "]: " << m_pathMetrics[predPMIxB]
//                    << " bitB: " << (int) bitB
//                    << " codeB: " << (int) codeB
//                    << " bmB: " << (int) bmB
//                    << " pmB: " << pmB << std::endl;

            bool a_b; // true: A, false: B

            if (pmA == pmB) // path metrics are even
            {
                if (bmA == bmB)
                {
                    a_b = true; // arbitrary
                }
                else
                {
                    a_b = bmA < bmB;
                }
            }
            else if (pmA < 0) // A infinite
            {
                a_b = false;
            }
            else if (pmB < 0) // B infinite
            {
                a_b = true;
            }
            else
            {
                a_b = pmA < pmB;
            }

            if (a_b) // A selected
            {
                m_pathMetrics[ib + (is+1)*(1<<(m_k-1))] = pmA;
                m_traceback[ib + is*(1<<(m_k-1))] = (predA<<1) + bitA; // Pack predecessor branch # and bit value

                if ((pmA >= 0) && (pmA < minMetric))
                {
                    minMetric = pmA;
                    minPathIndex = ib;
                }

//                std::cerr << "    Select A:"
//                        << " pm: " << pmA
//                        << " bit: " << (int) bitA
//                      << " m_traceback[" << ib + is*(1<<(m_k-1)) << "]: " << (predA<<1) + bitA
//                        << std::endl;
            }
            else
            {
                m_pathMetrics[ib + (is+1)*(1<<(m_k-1))] = pmB;
                m_traceback[ib + is*(1<<(m_k-1))] = (predB<<1) + bitB; // Pack predecessor branch # and bit value

                if ((pmB >= 0) && (pmB < minMetric))
                {
                    minMetric = pmB;
                    minPathIndex = ib;
                }

//                std::cerr << "    Select B:"
//                        << " pm: " << pmB
//                        << " bit: " << (int) bitB
//                      << " m_traceback[" << ib + is*(1<<(m_k-1)) << "]: " << (predB<<1) + bitB
//                        << std::endl;

            } // path decision
        } // branches
    } // symbols

    // trace back

    unsigned int bIx = minPathIndex;

    for (int is = nbSymbols-1; is >= 0; is--)
    {
//      std::cerr << "is: " << is
//              << " bIx: " << (int) bIx
//              << " bit: " << (int)  (m_traceback[bIx + is*(1<<(m_k-1))] % 2)
//              << " pred: " << (int) (m_traceback[bIx + is*(1<<(m_k-1))] >> 1)
//              << std::endl;
        dataBits[is] = m_traceback[bIx + is*(1<<(m_k-1))] % 2; // unpack bit value
        bIx = m_traceback[bIx + is*(1<<(m_k-1))] >> 1;         // unpack predecessor branch #
    }
}

void Viterbi3::doMetrics(
        int n,
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
    metric[0] = NbOnes[m_branchCodes[0] ^ symbol]; // 00    0
    metric[1] = NbOnes[m_branchCodes[1] ^ symbol]; // 00    1
    metric[2] = NbOnes[m_branchCodes[2] ^ symbol]; // 01    0
    metric[3] = NbOnes[m_branchCodes[3] ^ symbol]; // 01    1
    metric[4] = NbOnes[m_branchCodes[4] ^ symbol]; // 10    0
    metric[5] = NbOnes[m_branchCodes[5] ^ symbol]; // 10    1
    metric[6] = NbOnes[m_branchCodes[6] ^ symbol]; // 11    0
    metric[7] = NbOnes[m_branchCodes[7] ^ symbol]; // 11    1

    // This hardcodes the Treillis structure:

    // Pres. state = S0, Prev. state = S0 & S1
    m1 = metric[0] + m_pathMetric[0];
    m2 = metric[2] + m_pathMetric[1];
    if (m1 < m2)
    {
        m_pathMemory0[n] = 0;
        tempMetric[0] = m1;
    }
    else
    {
        m_pathMemory0[n] = 1;
        tempMetric[0] = m2;
    }; // end else - if

    // Pres. state = S1, Prev. state = S2 & S3
    m1 = metric[4] + m_pathMetric[2];
    m2 = metric[6] + m_pathMetric[3];
    if (m1 < m2)
    {
        m_pathMemory1[n] = 0;
        tempMetric[1] = m1;
    }
    else
    {
        m_pathMemory1[n] = 1;
        tempMetric[1] = m2;
    }; // end else - if

    // Pres. state = S2, Prev. state = S0 & S1
    m1 = metric[1] + m_pathMetric[0];
    m2 = metric[3] + m_pathMetric[1];
    if (m1 < m2)
    {
        m_pathMemory2[n] = 0;
        tempMetric[2] = m1;
    }
    else
    {
        m_pathMemory2[n] = 1;
        tempMetric[2] = m2;
    }

    // Pres. state = S3, Prev. state = S2 & S3
    m1 = metric[5] + m_pathMetric[2];
    m2 = metric[7] + m_pathMetric[3];
    if (m1 < m2)
    {
        m_pathMemory3[n] = 0;
        tempMetric[3] = m1;
    }
    else
    {
        m_pathMemory3[n] = 1;
        tempMetric[3] = m2;
    }; // end else - if

    // Store new path metrics

    m_pathMetric[0] = tempMetric[0];
    m_pathMetric[1] = tempMetric[1];
    m_pathMetric[2] = tempMetric[2];
    m_pathMetric[3] = tempMetric[3];

} // end function ViterbiDecode


}


