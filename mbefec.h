///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// This is a C++ adaptation of the ecc.h/ecc.c in mbelib                         //
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

#ifndef MBEFEC_H_
#define MBEFEC_H_

namespace DSDcc
{

/**
 * This is the Golay(23,11) used in AMBE FEC
 */
class GolayMBE
{
public:
    static int  mbe_golay2312(unsigned char *in, unsigned char *out);

private:
    static void mbe_checkGolayBlock(long int *block);

    static const int golayGenerator[12];
    static const int golayMatrix[2048];
};

/**
 * This is the Hamming(15,11) used in AMBE FEC
 */
class HammingMBE
{
public:
    static int mbe_hamming1511(unsigned char *in, unsigned char *out);
    static int mbe_7100x4400hamming1511(unsigned char *in, unsigned char *out);

private:
    static const int hammingGenerator[4];
    static const int imbe7100x4400hammingGenerator[4];
    static const int hammingMatrix[16];
};

} // namespace DSDcc

#endif /* MBEFEC_H_ */
