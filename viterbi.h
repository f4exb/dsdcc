///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// This is largely inspired by Phil Karn, KA9Q code                              //
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

#ifndef VITERBI_H_
#define VITERBI_H_

namespace DSDcc
{

class Viterbi
{
public:
    Viterbi(int k, int n, unsigned int *polys);
    ~Viterbi();

    /** Convolutionally encode data into binary symbols */
    int encode(
        unsigned char *symbols,
        unsigned char *data,
        unsigned int nbytes,
        unsigned int startstate,
        unsigned int endstate
    );

    /* Viterbi decoder */
    int viterbi(
        unsigned long *metric,      //!< Final path metric (returned value)
        unsigned char *data,        //!< Decoded output data
        unsigned char *symbols,     //!< Raw deinterleaved input symbols
        unsigned int nbits,         //!< Number of output bits
        int mettab[2][256],         //!< Metric table, [sent sym][rx symbol]
        unsigned int startstate,    //!< Encoder starting state
        unsigned int endstate       //!< Encoder ending state
    );

    static const unsigned int Poly23[];
    static const unsigned int Poly24[];
    static const unsigned int Poly25[];
    static const unsigned int Poly25a[];
    static const unsigned int Poly25y[];
    static const unsigned char Partab[];

private:
    static inline int parity(int x)
    {
      x ^= (x >> 16);
      x ^= (x >> 8);
      return Partab[x & 0xff];
    }

    int m_k;
    int m_n;
    unsigned int *m_polys;
    int m_d;
    int *m_syms;
    int m_vdInt;
};

} // namespace DSDcc

#endif /* VITERBI_H_ */
