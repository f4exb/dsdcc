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

#ifndef DSD_SYMBOL_H_
#define DSD_SYMBOL_H_

#include "dsd_filters.h"

namespace DSDcc
{

class DSDDecoder;

class DSDSymbol
{
public:
    DSDSymbol(DSDDecoder *dsdDecoder);
    ~DSDSymbol();

    int getSymbol() const { return m_symbol; }
    bool pushSample(short sample, int have_sync); //!< push a new sample into the decoder. Returns true if a new symbol is available
    int getDibit(); //!< from the last retrieved symbol Returns either the bit (0,1) or the dibit value (0,1,2,3)
    void print_datascope(int* sbuf2);

    static void compressBits(const char *bitArray, unsigned char *byteArray, int nbBytes)
    {
        for (int i = 0; i < nbBytes; i++)
        {
            byteArray[i] =  (1 & bitArray[8*i+0]) << 7;
            byteArray[i] += (1 & bitArray[8*i+1]) << 6;
            byteArray[i] += (1 & bitArray[8*i+2]) << 5;
            byteArray[i] += (1 & bitArray[8*i+3]) << 4;
            byteArray[i] += (1 & bitArray[8*i+4]) << 3;
            byteArray[i] += (1 & bitArray[8*i+5]) << 2;
            byteArray[i] += (1 & bitArray[8*i+6]) << 1;
            byteArray[i] += (1 & bitArray[8*i+7]) << 0;
        }

//        for (int i = nbBytes-1; i >= 0; i--)
//        {
//            byteArray[i] =  (1 & bitArray[8*i+7]) << 7;
//            byteArray[i] += (1 & bitArray[8*i+6]) << 6;
//            byteArray[i] += (1 & bitArray[8*i+5]) << 5;
//            byteArray[i] += (1 & bitArray[8*i+4]) << 4;
//            byteArray[i] += (1 & bitArray[8*i+4]) << 3;
//            byteArray[i] += (1 & bitArray[8*i+2]) << 2;
//            byteArray[i] += (1 & bitArray[8*i+1]) << 1;
//            byteArray[i] += (1 & bitArray[8*i+0]) << 0;
//        }
    }

private:
    void resetSymbol();
    int get_dibit_and_analog_signal(int* out_analog_signal);
    void use_symbol(int symbol);
    int digitize(int symbol);
    int invert_dibit(int dibit);

    DSDDecoder *m_dsdDecoder;
    DSDFilters m_dsdFilters;

    int m_symbol;      //!< the last retrieved symbol
    int m_sampleIndex; //!< the current sample index for the symbol in progress
    int m_sum;
    int m_count;

};

} // namespace DSDcc

#endif /* DSD_SYMBOL_H_ */
