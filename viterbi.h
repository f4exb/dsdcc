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
    Viterbi(int k, int n, const unsigned int *polys, bool msbFirst = true);
    ~Viterbi();

    /** Convolutionally encode data into binary symbols */
    void encodeToSymbols(
        unsigned char *symbols,
        const unsigned char *dataBits,
        unsigned int nbBits,
        unsigned int startstate
    );

    /** Convolutionally encode data into bits */
    void encodeToBits(
        unsigned char *codedBits,
        const unsigned char *dataBits,
        unsigned int nbBits,
        unsigned int startstate
    );

    /* Viterbi decoder */
    void decodeFromSymbols(
        unsigned char *dataBits,    //!< Decoded output data bits
        const unsigned char *symbols,     //!< Input symbols
        unsigned int nbSymbols,     //!< Number of imput symbols
        unsigned int startstate     //!< Encoder starting state
    );

    const unsigned char *getBranchCodes() const { return m_branchCodes; }
    const unsigned char *getPredA() const { return m_predA; }
    const unsigned char *getPredB() const { return m_predB; }
    const unsigned char *getBitA() const { return m_bitA; }
    const unsigned char *getBitB() const { return m_bitB; }

    static const unsigned int Poly23[];  //!< MIT lecture example
    static const unsigned int Poly23a[]; //!< D-Star
    static const unsigned int Poly24[];
    static const unsigned int Poly25[];
    static const unsigned int Poly25a[];
    static const unsigned int Poly25y[]; //!< Yaesu System Fusion
    static const unsigned char Partab[];

private:
    void initCodes();
    void initTreillis();
    static inline int parity(int x)
    {
      x ^= (x >> 16);
      x ^= (x >> 8);
      return Partab[x & 0xff];
    }

    int m_k;
    int m_n;
    const unsigned int *m_polys;
    bool m_msbFirst;
    int *m_pathMetrics;
    unsigned char *m_paths;
    unsigned char *m_branchCodes;
    unsigned char *m_predA;
    unsigned char *m_bitA;
    unsigned char *m_predB;
    unsigned char *m_bitB;
    int m_nbSymbolsMax;
};

} // namespace DSDcc

#endif /* VITERBI_H_ */
