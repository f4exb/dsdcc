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

#ifndef PN_H_
#define PN_H_

namespace DSDcc
{

class PN_9_5
{
public:
    PN_9_5(unsigned int seed);
    ~PN_9_5();

    unsigned char getByte(unsigned int byteIndex) const
    {
        return m_byteTable[byteIndex % 64];
    }

    unsigned char getBit(unsigned int bitIndex) const
    {
        return m_bitTable[bitIndex % 512];
    }

    const unsigned char *getBits() const
    {
        return m_bitTable;
    }

private:
    void init();

    unsigned int m_seed;
    unsigned char m_byteTable[64];
    unsigned char m_bitTable[512];
};

} // namespace DSDcc

#endif /* PN_H_ */
