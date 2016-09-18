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

#ifndef YSF_H_
#define YSF_H_

#include <string>

#include "viterbi5.h"
#include "fec.h"

namespace DSDcc
{

class DSDDecoder;

class DSDYSF
{
public:
    DSDYSF(DSDDecoder *dsdDecoder);
    ~DSDYSF();

    void init();
    void process();

private:
    void processFICH(int symbolIndex, unsigned char dibit);
    bool checkCRC16(unsigned char *bits, int nbBits);

    DSDDecoder *m_dsdDecoder;
    int m_symbolIndex;                //!< Current symbol index
    unsigned char m_fichRaw[100];     //!< FICH dibits after de-interleave + Viterbi stuff symbols
    unsigned char m_fichGolay[100];   //!< FICH Golay encoded bits + 4 stuff bits + Viterbi stuff bits
    unsigned char m_fich[48];         //!< Final FICH + CRC16
    unsigned char m_fichCRC[16];      //!< Calculated CRC
    Viterbi5 m_viterbiFICH;
    Golay_24_12 m_golay_24_12;
    unsigned char m_bitWork[48];

    static const int m_fichInterleave[100]; //!<
};

} // namespace DSDcc



#endif /* YSF_H_ */
