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
    void encodeToSymbols(
        unsigned char *symbols,
        unsigned char *dataBits,
        unsigned int nbBits,
        unsigned int startstate
    );

    /** Convolutionally encode data into bits */
    void encodeToBits(
        unsigned char *codedBits,
        unsigned char *dataBits,
        unsigned int nbBits,
        unsigned int startstate
    );

    /* Viterbi decoder */
    void decodeFromSymbols(
        unsigned char *dataBits,    //!< Decoded output data bits
        unsigned char *symbols,     //!< Input symbols
        unsigned int nbSymbols,     //!< Number of imput symbols
        unsigned int startstate     //!< Encoder starting state
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
    int *m_pathMetrics;
    unsigned char *m_paths;
    int m_nbSymbolsMax;
};

} // namespace DSDcc

#endif /* VITERBI_H_ */
