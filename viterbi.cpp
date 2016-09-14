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

#include <string.h>
#include "viterbi.h"

namespace DSDcc
{

const unsigned int Viterbi::Poly23[] = { 0x7, 0x5 };
const unsigned int Viterbi::Poly24[] = { 0xf, 0xb };
const unsigned int Viterbi::Poly25[] = { 0x17, 0x19 };
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


Viterbi::Viterbi(int k, int n, unsigned int *polys) :
        m_k(k),
        m_n(n),
        m_polys(polys),
        m_nbSymbolsMax(0)
{
    m_pathMetrics = new int[(1<<m_k) - 1];
    m_paths = 0;
}

Viterbi::~Viterbi()
{
    delete[] m_pathMetrics;

    if (m_paths) {
        delete[] m_paths;
    }
}

void Viterbi::encodeToSymbols(
        unsigned char *symbols,
        unsigned char *dataBits,
        unsigned int nbBits,
        unsigned int startstate)
{
    int i, j;
    unsigned int encstate = startstate;

    for (int i = 0; i < nbBits; nbBits++)
    {
        encstate = (encstate >> 1) | (dataBits[i] << (m_k-1));
        *symbols = 0;

        for (j = 0; j < m_n; j++)
        {
            *symbols += parity(encstate & m_polys[j]) << j;
        }

        symbols++;
    }
}

void Viterbi::encodeToBits(
        unsigned char *codedBits,
        unsigned char *dataBits,
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
        unsigned char *dataBits,    //!< Decoded output data bits
        unsigned char *symbols,     //!< Input symbols
        unsigned int nbSymbols,     //!< Number of imput symbols
        unsigned int startstate     //!< Encoder starting state
)
{
    if (nbSymbols > m_nbSymbolsMax)
    {
        if (m_paths) {
            delete[] m_paths;
        }

        m_paths = new unsigned char[((1<<m_k) - 1) * nbSymbols];
        m_nbSymbolsMax = nbSymbols;
    }


}



} // namespace DSDcc


