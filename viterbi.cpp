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
        m_vdInt(0)
{
    if (k >= 6) {
        m_d = (1 << (k-6));
    } else {
        m_d = 1;
    }

    m_syms = new int[1<<k];
}

Viterbi::~Viterbi()
{
    delete[] m_syms;
}

int Viterbi::encode(
        unsigned char *symbols,
        unsigned char *data,
        unsigned int nbytes,
        unsigned int startstate,
        unsigned int endstate)
{
    int i, j;
    unsigned int encstate = startstate;

    while (nbytes-- != 0)
    {
        for (i = 7; i >= 0; i--)
        {
            encstate = (encstate << 1) | ((*data >> i) & 1);

            for (j = 0; j < m_n; j++)
            {
                *symbols++ = parity(encstate & m_polys[j]);
            }
        }
        data++;
    }

    /* Flush out tail */
    for (i = m_k - 2; i >= 0; i--)
    {
        encstate = (encstate << 1) | ((endstate >> i) & 1);

        for (j = 0; j < m_n; j++)
        {
            *symbols++ = parity(encstate & m_polys[j]);
        }
    }

    return 0;
}

int Viterbi::viterbi(
        unsigned long *metric,
        unsigned char *data,
        unsigned char *symbols,
        unsigned int nbits,
        int mettab[2][256],
        unsigned int startstate,
        unsigned int endstate
)
{
    int bitcnt = -(m_k - 1);
    long m0, m1;
    int i, j, sym;
    int mets[1 << m_n];
    unsigned long paths[(nbits + m_k - 1) * m_d], *pp, mask;
    long cmetric[1 << (m_k - 1)], nmetric[1 << (m_k - 1)];

    memset(paths, 0, sizeof(paths));

    if (!m_vdInt)
    {
        for (i = 0; i < (1 << m_k); i++)
        {
            sym = 0;

            for (j = 0; j < m_n; j++)
            {
                sym = (sym << 1) + parity(i & m_polys[j]);
            }

            m_syms[i] = sym;
        }

        m_vdInt++;
    }

    startstate &= ~((1 << (m_k - 1)) - 1);
    endstate &= ~((1 << (m_k - 1)) - 1);

    /* Initialize starting metrics */

    for (i = 0; i < 1 << (m_k - 1); i++)
    {
        cmetric[i] = -999999;
    }

    cmetric[startstate] = 0;
    pp = paths;

    for (;;)
    { /* For each data bit */
        /* Read input symbols and compute branch metrics */
        for (i = 0; i < 1 << m_n; i++)
        {
            mets[i] = 0;
            for (j = 0; j < m_n; j++)
            {
                mets[i] += mettab[(i >> (m_n - j - 1)) & 1][symbols[j]];
            }
        }

        symbols += m_n;
        /* Run the add-compare-select operations */
        mask = 1;

        for (i = 0; i < 1 << (m_k - 1); i += 2)
        {
            int b1, b2;

            b1 = mets[m_syms[i]];
            nmetric[i] = m0 = cmetric[i / 2] + b1;
            b2 = mets[m_syms[i + 1]];
            b1 -= b2;
            m1 = cmetric[(i / 2) + (1 << (m_k - 2))] + b2;

            if (m1 > m0)
            {
                nmetric[i] = m1;
                *pp |= mask;
            }

            m0 -= b1;
            nmetric[i + 1] = m0;
            m1 += b1;

            if (m1 > m0)
            {
                nmetric[i + 1] = m1;
                *pp |= mask << 1;
            }

            mask <<= 2;

            if (mask == 0)
            {
                mask = 1;
                pp++;
            }
        }
        if (mask != 1)
        {
            pp++;
        }
        if (++bitcnt == nbits)
        {
            *metric = nmetric[endstate];
            break;
        }

        memcpy(cmetric, nmetric, sizeof(cmetric));
    }

    /* Chain back from terminal state to produce decoded data */
    if (data == 0)
    {
        return 0;/* Discard output */
    }

    memset(data, 0, (nbits + 7) / 8); /* round up in case nbits % 8 != 0 */

    for (i = nbits - 1; i >= 0; i--)
    {
        pp -= m_d;
        if (pp[endstate >> 5] & (1 << (endstate & 31)))
        {
            endstate |= (1 << (m_k - 1));
            data[i >> 3] |= 0x80 >> (i & 7);
        }
        endstate >>= 1;
    }
    return 0;
}



} // namespace DSDcc


