///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// Class specialized for K=5 decoding                                            //
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

#ifndef VITERBI5_H_
#define VITERBI5_H_

#include "viterbi.h"
#include "export.h"

namespace DSDcc
{

class DSDCC_API Viterbi5 : public Viterbi
{
public:
    Viterbi5(int n, const unsigned int *polys, bool msbFirst = true);
    virtual ~Viterbi5();

    /* Viterbi decoder */
    virtual void decodeFromSymbols(
            unsigned char *dataBits,    //!< Decoded output data bits
            const unsigned char *symbols,     //!< Input symbols
            unsigned int nbSymbols,     //!< Number of imput symbols
            unsigned int startstate     //!< Encoder starting state
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
            unsigned char *m_pathMemory4,
            unsigned char *m_pathMemory5,
            unsigned char *m_pathMemory6,
            unsigned char *m_pathMemory7,
            unsigned char *m_pathMemory8,
            unsigned char *m_pathMemory9,
            unsigned char *m_pathMemory10,
            unsigned char *m_pathMemory11,
            unsigned char *m_pathMemory12,
            unsigned char *m_pathMemory13,
            unsigned char *m_pathMemory14,
            unsigned char *m_pathMemory15,
            uint32_t *m_pathMetric
    );

    static void traceBack (
            int nbSymbols,
            unsigned int startState,
            unsigned char *out,
            unsigned char *m_pathMemory0,
            unsigned char *m_pathMemory1,
            unsigned char *m_pathMemory2,
            unsigned char *m_pathMemory3,
            unsigned char *m_pathMemory4,
            unsigned char *m_pathMemory5,
            unsigned char *m_pathMemory6,
            unsigned char *m_pathMemory7,
            unsigned char *m_pathMemory8,
            unsigned char *m_pathMemory9,
            unsigned char *m_pathMemory10,
            unsigned char *m_pathMemory11,
            unsigned char *m_pathMemory12,
            unsigned char *m_pathMemory13,
            unsigned char *m_pathMemory14,
            unsigned char *m_pathMemory15
    );
};

} // namespace DSDcc

#endif /* VITERBI5_H_ */
