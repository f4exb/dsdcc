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

#ifndef DSDCC_DESCRAMBLE_H_
#define DSDCC_DESCRAMBLE_H_

namespace DSDcc
{

class Descramble
{
public:
    static void scramble (unsigned char *in, unsigned char *out);
    static void deinterleave (unsigned char *in, unsigned char *out);
    static int FECdecoder (unsigned char *in, unsigned char *out);

private:
    static int traceBack (unsigned char *out,
            unsigned char *m_pathMemory0,
            unsigned char *m_pathMemory1,
            unsigned char *m_pathMemory2,
            unsigned char *m_pathMemory3);
    static void viterbiDecode (int n,
            unsigned char *data,
            unsigned char *m_pathMemory0,
            unsigned char *m_pathMemory1,
            unsigned char *m_pathMemory2,
            unsigned char *m_pathMemory3,
            unsigned char *m_pathMetric);

    static const int SCRAMBLER_TABLE_BITS_LENGTH=720;
    static const unsigned char SCRAMBLER_TABLE_BITS[];
};


} // namespace DSDcc

#endif /* DSDCC_DESCRAMBLE_H_ */
