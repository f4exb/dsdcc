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
#include <limits.h>
#include "viterbi.h"

namespace DSDcc
{

const unsigned int Viterbi::Poly23[]  = {  0x7,  0x6 };
const unsigned int Viterbi::Poly23a[] = {  0x7,  0x5 };
const unsigned int Viterbi::Poly24[]  = {  0xf,  0xb };
const unsigned int Viterbi::Poly25[]  = { 0x17, 0x19 };
const unsigned int Viterbi::Poly25a[] = { 0x13, 0x1b };
const unsigned int Viterbi::Poly25y[] = { 0x13, 0x1d };

const unsigned char Viterbi::Partab[] = {
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0,
};

// calculated with:
//int NumberOfSetBits(int i)
//{
//     // Java: use >>> instead of >>
//     // C or C++: use uint32_t
//     i = i - ((i >> 1) & 0x55555555);
//     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
//     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
//}
const unsigned char Viterbi::NbOnes[] = {
		0, 1, 1, 2, 1, 2, 2, 3,
		1, 2, 2, 3, 2, 3, 3, 4,
		1, 2, 2, 3, 2, 3, 3, 4,
		2, 3, 3, 4, 3, 4, 4, 5,
		1, 2, 2, 3, 2, 3, 3, 4,
		2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5,
		3, 4, 4, 5, 4, 5, 5, 6,
		1, 2, 2, 3, 2, 3, 3, 4,
		2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5,
		3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5,
		3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6,
		4, 5, 5, 6, 5, 6, 6, 7,
		1, 2, 2, 3, 2, 3, 3, 4,
		2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5,
		3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5,
		3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6,
		4, 5, 5, 6, 5, 6, 6, 7,
		2, 3, 3, 4, 3, 4, 4, 5,
		3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6,
		4, 5, 5, 6, 5, 6, 6, 7,
		3, 4, 4, 5, 4, 5, 5, 6,
		4, 5, 5, 6, 5, 6, 6, 7,
		4, 5, 5, 6, 5, 6, 6, 7,
		5, 6, 6, 7, 6, 7, 7, 8,
};

const uint32_t Viterbi::m_maxMetric = 0xFFFE0000; // Account for any possible accumulation of branch metrics


Viterbi::Viterbi(int k, int n, const unsigned int *polys, bool msbFirst) :
        m_k(k),
        m_n(n),
        m_polys(polys),
        m_nbSymbolsMax(0),
        m_nbBitsMax(0),
		m_msbFirst(msbFirst)
{
    m_branchCodes = new unsigned char[(1<<m_k)];
    m_predA = new unsigned char[1<<(m_k-1)];
    m_predB = new unsigned char[1<<(m_k-1)];
    m_pathMetrics = 0;
    m_traceback = 0;
    m_symbols = 0;

    initCodes();
    initTreillis();
}

Viterbi::~Viterbi()
{
    if (m_symbols) {
        delete[] m_symbols;
    }

    if (m_pathMetrics) {
        delete[] m_pathMetrics;
    }

    if (m_traceback) {
        delete[] m_traceback;
    }

    delete[] m_predB;
    delete[] m_predA;
	delete[] m_branchCodes;
}

void Viterbi::initCodes()
{
	unsigned char symbol;
	unsigned char dataBit;

	for (int i = 0; i < (1<<(m_k-1)); i++)
	{
		dataBit = 0;
		encodeToSymbols(&symbol, &dataBit, 1, i<<1);
		m_branchCodes[2*i] = symbol;
        dataBit = 1;
        encodeToSymbols(&symbol, &dataBit, 1, i<<1);
        m_branchCodes[2*i+1] = symbol;
	}
}

void Viterbi::initTreillis()
{
    for (unsigned int s = 0; s < 1<<(m_k-2); s++)
    {
        m_predA[s] = (s<<1);
        m_predB[s] = (s<<1) + 1;
        m_predA[s+(1<<(m_k-2))] = (s<<1);
        m_predB[s+(1<<(m_k-2))] = (s<<1) + 1;
    }
}

void Viterbi::encodeToSymbols(
        unsigned char *symbols,
        const unsigned char *dataBits,
        unsigned int nbBits,
        unsigned int startstate)
{
    unsigned int encstate = startstate;

    for (int i = 0; i < nbBits; i++)
    {
        encstate = (encstate >> 1) | (dataBits[i] << (m_k-1));
        *symbols = 0;

        for (int j = 0; j < m_n; j++)
        {
            *symbols += parity(encstate & m_polys[j]) << (m_msbFirst ? (m_n - 1) - j : j);
        }

        symbols++;
    }
}

void Viterbi::encodeToBits(
        unsigned char *codedBits,
        const unsigned char *dataBits,
        unsigned int nbBits,
        unsigned int startstate)
{
    int i, j;
    unsigned int encstate = startstate;

    for (int i = 0; i < nbBits; i++)
    {
        encstate = (encstate >> 1) | (dataBits[i] << (m_k-1));

        for (j = 0; j < m_n; j++)
        {
            *codedBits = parity(encstate & m_polys[j]);
            codedBits++;
        }
    }
}

void Viterbi::decodeFromBits(
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

void Viterbi::decodeFromSymbols(
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

        m_traceback = new unsigned char[(1<<(m_k-1)) * nbSymbols];
        m_pathMetrics = new uint32_t[(1<<(m_k-1))*2]; // only one step back in memory
        m_nbSymbolsMax = nbSymbols;
    }

    // initial path metrics state
    memset(m_pathMetrics, m_maxMetric, sizeof(uint32_t) * (1<<(m_k-1)));
    m_pathMetrics[startstate] = 0;

    unsigned int minPathIndex;
    unsigned int minMetric;

    for (int is = 0; is < nbSymbols; is++)
    {
        minMetric = m_maxMetric;
//        std::cerr << "S[" << is << "]=" << (int) symbols[is] << std::endl;

        // compute branch metrics
    	for (unsigned int ib = 0; ib < 1<<(m_k-1); ib++)
    	{
            unsigned char bit = ib < 1<<(m_k-2) ? 0 : 1;

            // path A

    	    unsigned char predA = m_predA[ib];
    	    unsigned int predPMIxA = (is%2)*(1<<(m_k-1)) + predA;
    	    unsigned char codeA = m_branchCodes[(predA<<1)+bit];
    	    unsigned char bmA = NbOnes[codeA ^ symbols[is]]; // branch metric
    	    unsigned int pmA = m_pathMetrics[predPMIxA] + bmA; // add branch metric to path metric

            // path B

    	    unsigned char predB = m_predB[ib];
            unsigned int predPMIxB = (is%2)*(1<<(m_k-1)) + predB;
            unsigned char codeB = m_branchCodes[(predB<<1)+bit];
            unsigned char bmB = NbOnes[codeB ^ symbols[is]]; // branch metric
            unsigned int pmB = m_pathMetrics[predPMIxB] + bmB; // add branch metric to path metric

            // decisions, decisions ...

//            std::cerr << "  Branch:"
//                    << " " << ib
//                    << " bit: " << (int) bit
//                    << " predA: " << (int) predA
//                    << " pm[" << (int) predA << ":" << is*(1<<(m_k-1)) << "]: " << m_pathMetrics[predPMIxA]
//                    << " codeA: " << (int) codeA
//                    << " bmA: " << (int) bmA
//                    << " pmA: " << pmA
//                    << " | predB: " << (int) predB
//                    << " pm[" << (int) predB << ":" << is*(1<<(m_k-1)) << "]: " << m_pathMetrics[predPMIxB]
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
            else
            {
                a_b = pmA < pmB;
            }

            if (a_b) // A selected
            {
                m_pathMetrics[ib + ((is+1)%2)*(1<<(m_k-1))] = pmA;
                m_traceback[ib + is*(1<<(m_k-1))] = predA; // Pack predecessor branch # and bit value

                if (pmA < minMetric)
                {
                    minMetric = pmA;
                    minPathIndex = ib;
                }

//                std::cerr << "    Select A:"
//                        << " pm: " << pmA
//                        << " bit: " << (int) bit
//                        << " m_traceback[" << ib + is*(1<<(m_k-1)) << "]: " << (int) predA
//                        << std::endl;
            }
            else
            {
                m_pathMetrics[ib + ((is+1)%2)*(1<<(m_k-1))] = pmB;
                m_traceback[ib + is*(1<<(m_k-1))] = predB; // Pack predecessor branch # and bit value

                if (pmB < minMetric)
                {
                    minMetric = pmB;
                    minPathIndex = ib;
                }

//                std::cerr << "    Select B:"
//                        << " pm: " << pmB
//                        << " bit: " << (int) bit
//                        << " m_traceback[" << ib + is*(1<<(m_k-1)) << "]: " << (int) predB
//                        << std::endl;

            } // path decision
    	} // branches
    } // symbols

    // trace back

    unsigned int bIx = minPathIndex;

    for (int is = nbSymbols-1; is >= 0; is--)
    {
//    	std::cerr << "is: " << is
//    			<< " bIx: " << (int) bIx
//    			<< " bit: " << bIx < 1<<(m_k-2) ? 0 : 1;
//				<< " pred: " << (int) m_traceback[bIx + is*(1<<(m_k-1))]
//				<< std::endl;
        dataBits[is] = bIx < 1<<(m_k-2) ? 0 : 1;
        bIx = m_traceback[bIx + is*(1<<(m_k-1))];
    }
}


} // namespace DSDcc


