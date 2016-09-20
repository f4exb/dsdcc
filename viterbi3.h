///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// Class specialized for K=3 decoding                                            //
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

#ifndef VITERBI3_H_
#define VITERBI3_H_

#include "viterbi.h"

namespace DSDcc
{

class Viterbi3 : public Viterbi
{
public:
    Viterbi3(int n, const unsigned int *polys, bool msbFirst = true);
    virtual ~Viterbi3();

    /* Viterbi decoder */
    virtual void decodeFromSymbols(
            unsigned char *dataBits,      //!< Decoded output data bits
            const unsigned char *symbols, //!< Input symbols
            unsigned int nbSymbols,       //!< Number of imput symbols
            unsigned int startstate       //!< Encoder starting state
    );

    /* Viterbi decoder */
    virtual void decodeFromBits(
        unsigned char *dataBits,    //!< Decoded output data bits
        const unsigned char *bits,  //!< Input bits
        unsigned int nbBits,        //!< Number of imput bits
        unsigned int startstate     //!< Encoder starting state
    );

private:
    static void doMetrics (
            int n,
            unsigned char *branchCodes,
            unsigned char symbol,
            unsigned char *m_pathMemory0,
            unsigned char *m_pathMemory1,
            unsigned char *m_pathMemory2,
            unsigned char *m_pathMemory3,
            uint32_t *m_pathMetric
    );

    static void traceBack (
            int nbSymbols,
            unsigned int startState,
            unsigned char *out,
            unsigned char *m_pathMemory0,
            unsigned char *m_pathMemory1,
            unsigned char *m_pathMemory2,
            unsigned char *m_pathMemory3
    );
};

} // namespace DSDcc

#endif /* VITERBI3_H_ */
