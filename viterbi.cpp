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

const unsigned char Viterbi::NbOnes[] = {
        0, 1, 1, 2, 1, 2, 2, 3, //  7
        1, 2, 2, 3, 2, 3, 3, 4, // 15
        1, 2, 2, 3, 2, 3, 3, 4, // 23
        2, 3, 3, 4, 3, 4, 4, 5, // 31
        1, 2, 2, 3, 2, 3, 3, 4, // 39
        2, 3, 3, 4, 3, 4, 4, 5, // 47
        2, 3, 3, 4, 3, 4, 5, 5, // 55
        3, 4, 4, 5, 4, 5, 4, 6, // 63
        1, 2, 2, 3, 2, 3, 3, 4, // 71
        2, 3, 3, 4, 3, 4, 4, 5, // 79
        2, 3, 3, 4, 3, 4, 4, 5, // 87
        3, 4, 4, 5, 4, 5, 5, 6, // 95
        2, 3, 3, 4, 3, 4, 4, 5, // 103
        3, 4, 4, 5, 4, 5, 5, 6, // 111
        3, 4, 4, 5, 4, 5, 5, 6, // 119
        4, 5, 5, 6, 5, 6, 6, 7, // 127
        1, 2, 2, 3, 2, 3, 3, 4, // 135
        2, 3, 3, 4, 3, 4, 4, 5, // 143
        2, 3, 3, 4, 3, 4, 4, 5, // 151
        3, 4, 4, 5, 4, 5, 5, 6, // 159
        2, 3, 3, 4, 3, 4, 4, 5, // 167
        3, 4, 4, 5, 4, 5, 5, 6, // 175
        3, 4, 4, 5, 4, 5, 5, 6, // 183
        4, 5, 5, 6, 5, 6, 6, 7, // 191
        2, 3, 3, 4, 3, 4, 4, 5, // 199
        3, 4, 4, 5, 4, 5, 5, 6, // 207
        3, 4, 4, 5, 4, 5, 5, 6, // 215
        4, 5, 5, 6, 5, 6, 6, 7, // 223
        3, 4, 4, 5, 4, 5, 5, 6, // 231
        4, 5, 5, 6, 5, 6, 6, 7, // 239
        4, 5, 5, 6, 5, 6, 6, 7, // 247
        5, 6, 6, 7, 6, 7, 7, 8, // 255
};


Viterbi::Viterbi(int k, int n, const unsigned int *polys, bool msbFirst) :
        m_k(k),
        m_n(n),
        m_polys(polys),
        m_nbSymbolsMax(0),
		m_msbFirst(msbFirst)
{
    m_branchCodes = new unsigned char[(1<<m_k)];
    m_predA = new unsigned char[1<<(m_k-1)];
    m_bitA  = new unsigned char[1<<(m_k-1)];
    m_predB = new unsigned char[1<<(m_k-1)];
    m_bitB  = new unsigned char[1<<(m_k-1)];
    m_pathMetrics = 0;
    m_paths = 0;

    initCodes();
    initTreillis();
}

Viterbi::~Viterbi()
{
    if (m_pathMetrics) {
        delete[] m_pathMetrics;
    }

    if (m_paths) {
        delete[] m_paths;
    }

    delete[] m_bitB;
    delete[] m_predB;
    delete[] m_bitA;
    delete[] m_predA;
	delete[] m_branchCodes;
}

void Viterbi::initCodes()
{
	unsigned char symbol;
	unsigned char dataBit;

	for (int i = 0; i < (1<<m_k); i++)
	{
		dataBit = i%2;
		encodeToSymbols(&symbol, &dataBit, 1, (i>>1)<<1);
		m_branchCodes[i] = symbol;
	}
}

void Viterbi::initTreillis()
{
    memset(m_bitA, 0xFF, 1<<(m_k-1));
    memset(m_bitB, 0xFF, 1<<(m_k-1));

	for (unsigned int s = 0; s < 1<<(m_k-1); s++)
	{
		unsigned char nextS = (s>>1);
        // branch A
		if (m_bitA[nextS] == 0xFF)
		{
	        m_predA[nextS] = s;
	        m_bitA[nextS] = 0;
		}
		// branch B
		else if (m_bitB[nextS] == 0xFF)
		{
	        m_predB[nextS] = s;
	        m_bitB[nextS] = 0;
		}
		else
		{
		    std::cout << "Viterbi::initTreillis: error at symbol " << (int) s << " (0)" << std::endl;
		}

		nextS += 1<<(m_k-2);
        // branch A
        if (m_bitA[nextS] == 0xFF)
        {
            m_predA[nextS] = s;
            m_bitA[nextS] = 1;
        }
        // branch B
        else if (m_bitB[nextS] == 0xFF)
        {
            m_predB[nextS] = s;
            m_bitB[nextS] = 1;
        }
        else
        {
            std::cout << "Viterbi::initTreillis: error at symbol " << (int) s << " (1)" << std::endl;
        }
	}
}

void Viterbi::encodeToSymbols(
        unsigned char *symbols,
        const unsigned char *dataBits,
        unsigned int nbBits,
        unsigned int startstate)
{
    int i, j;
    unsigned int encstate = startstate;

    for (int i = 0; i < nbBits; i++)
    {
        encstate = (encstate >> 1) | (dataBits[i] << (m_k-1));
        *symbols = 0;

        for (j = 0; j < m_n; j++)
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

    for (int i = 0; i < nbBits; nbBits++)
    {
        encstate = (encstate >> 1) | (dataBits[i] << (m_k-1));

        for (j = 0; j < m_n; j++)
        {
            *codedBits = parity(encstate & m_polys[j]);
            codedBits++;
        }
    }
}

void Viterbi::decodeFromSymbols(
        unsigned char *dataBits,      //!< Decoded output data bits
        const unsigned char *symbols, //!< Input symbols
        unsigned int nbSymbols,       //!< Number of imput symbols
        unsigned int startstate)      //!< Encoder starting state

{
    if (nbSymbols > m_nbSymbolsMax)
    {
        if (m_paths) {
            delete[] m_paths;
        }

        if (m_pathMetrics) {
            delete[] m_pathMetrics;
        }

        m_paths = new unsigned char[(1<<(m_k-1)) * nbSymbols];
        m_pathMetrics = new int[(1<<(m_k-1)) * (nbSymbols+1)];
        m_nbSymbolsMax = nbSymbols;
    }

    // initial path metrics state
    memset(m_pathMetrics, -1, sizeof(int) * (1<<(m_k-1)));
    m_pathMetrics[startstate] = 0;

    unsigned int minPathIndex;
    unsigned char minBit;

    for (int is = 0; is < nbSymbols; is++)
    {
        int minMetric = INT_MAX;
        std::cerr << "S[" << is << "]=" << (int) symbols[is] << std::endl;

        // compute branch metrics
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

            std::cerr << "  Branch:"
                    << " " << ib
                    << " predA: " << (int) predA
                    << " pm[" << predPMIxA << "]: " << m_pathMetrics[predPMIxA]
                    << " bitA: " << (int) bitA
                    << " codeA: " << (int) codeA
                    << " bmA: " << (int) bmA
                    << " pmA: " << pmA
                    << " | predB: " << (int) predB
                    << " pm[" << predPMIxB << "]: " << m_pathMetrics[predPMIxB]
                    << " bitB: " << (int) bitB
                    << " codeB: " << (int) codeB
                    << " bmB: " << (int) bmB
                    << " pmB: " << pmB << std::endl;

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
                m_paths[ib*nbSymbols + is] = bitA;

                if ((pmA >= 0) && (pmA < minMetric))
                {
                    minMetric = pmA;
                    minPathIndex = ib;
                    minBit = bitA;
                }

                std::cerr << "    Select A:"
                        << " pm: " << pmA
                        << " bit: " << (int) bitA
                        << std::endl;
            }
            else
            {
                m_pathMetrics[ib + (is+1)*(1<<(m_k-1))] = pmB;
                m_paths[ib*nbSymbols + is] = bitB;

                if ((pmB >= 0) && (pmB < minMetric))
                {
                    minMetric = pmB;
                    minPathIndex = ib;
                    minBit = bitB;
                }

                std::cerr << "    Select B:"
                        << " pm: " << pmB
                        << " bit: " << (int) bitB
                        << std::endl;

            }
    	} // branches

    	std::cerr << "  Selected branch:"
    	        << " index: " << minPathIndex
    	        << " bit: " << (int) minBit << std::endl;
    } // symbols

    memcpy(dataBits, &m_paths[minPathIndex*nbSymbols], nbSymbols);
}


} // namespace DSDcc


