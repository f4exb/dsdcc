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

#include "pn.h"

namespace DSDcc
{

PN_9_5::PN_9_5(unsigned int seed) : m_seed(seed)
{
    init();
}

PN_9_5::~PN_9_5()
{}


void PN_9_5::init()
{
    unsigned char byte;
    unsigned int sr = m_seed;

    for (int i = 0; i < 512; i++)
    {
        if (i%8  == 0)
        {
            byte = 0;
        }

        unsigned int bit0 = (sr & 1);
        unsigned int bit4 = (sr & 0x10) >> 4;
        sr >>= 1;
        sr |= (bit4 ^ bit0) << 8;

        m_bitTable[i] = bit0;
        byte += bit0 << (7 - (i%8));

        if (i%8 == 7)
        {
            m_byteTable[i/8] = byte;
        }
    }

}

} // namespace DSDcc
