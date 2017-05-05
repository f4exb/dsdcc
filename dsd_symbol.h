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
#include "doublebuffer.h"
#include "runningmaxmin.h"
#include "phaselock.h"

namespace DSDcc
{

class DSDDecoder;

class DSDSymbol
{
public:
    explicit DSDSymbol(DSDDecoder *dsdDecoder);
    ~DSDSymbol();

    void noCarrier();
    void resetFrameSync();

    void snapLevels(int nbSymbols); //!< take snapshot for min/max over a number of symbols
    void setSamplesPerSymbol(int samplesPerSymbol);
    void setFSK(unsigned int nbSymbols, bool inverted=false);
    void setNoSignal(bool noSignal) { m_noSignal = noSignal; }
    bool pushSample(short sample); //!< push a new sample into the decoder. Returns true if a new symbol is available

    int getSymbol() const { return m_symbol; }
    int getDibit(); //!< from the last retrieved symbol Returns either the bit (0,1) or the dibit value (0,1,2,3)
    unsigned char *getDibitBack(unsigned int shift) { return m_binSymbolBuffer.getBack(shift); }
    unsigned char *getSyncDibitBack(unsigned int shift) { return m_syncSymbolBuffer.getBack(shift); }
    unsigned char *getNonInvertedSyncDibitBack(unsigned int shift) { return m_nonInvertedSyncSymbolBuffer.getBack(shift); }

    static int invert_dibit(int dibit);
    int getLevel() const { return (m_max - m_min) / 328; }
    int getCarrierPos() const { return m_center / 164; }
    int getZeroCrossingPos() const { return m_zeroCrossingPos; }
    int getSymbolSyncQuality() const { return m_symbolSyncQuality; }
    short getFilteredSample() const { return m_filteredSample; }
    short getSymbolSyncSample() const { return m_symbolSyncSample; }
    int getSamplesPerSymbol() const { return m_samplesPerSymbol; }
    bool getPLLLocked() const { return m_pll.locked(); }

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
    }

private:
    void resetSymbol();
    void resetZeroCrossing();
    int get_dibit();
//    void use_symbol(int symbol);
    unsigned char digitize(int symbol);
    void digitizeIntoBinaryBuffer();
    void snapMinMax();
    static int comp(const void *a, const void *b);
    static int compShort(const void *a, const void *b);

    DSDDecoder *m_dsdDecoder;
    DSDFilters m_dsdFilters;

    int m_symbol;      //!< the last retrieved symbol
    int m_sampleIndex; //!< the current sample index for the symbol in progress
    int m_sum;
    int m_count;
    bool m_noSignal;   //!< for now used only for DMR mobile inbound when in silent slot

    int m_zeroCrossing;
    bool m_zeroCrossingInCycle;
    int m_zeroCrossingPos;
    int m_zeroCrossingCorrectionProfile[11];
    int m_zeroCrossingSlopeDivisor;

    int m_lbuf[32*2], m_lbuf2[32]; //!< symbol buffers for min/max
    int m_lmmidx;                  //!< index in min/max symbol buffer
    int m_min, m_max;
    int m_center;
    int m_umid, m_lmid;
    int m_numflips;
    int m_symbolSyncQuality;
    int m_symbolSyncQualityCounter;
    short m_lastsample;
    short m_filteredSample;
    short m_symbolSyncSample;
    unsigned int m_nbFSKSymbols;
    bool m_invertedFSK;
    int  m_samplesPerSymbol;
    lemiremaxmintruestreaming<short> m_lmmSamples;    //!< running min/max calculator
    DSDSecondOrderRecursiveFilter m_ringingFilter;
    SimplePhaseLock m_pll;
    DoubleBuffer<unsigned char> m_binSymbolBuffer;    //!< digitized symbol
    DoubleBuffer<unsigned char> m_syncSymbolBuffer;   //!< symbol digitized for synchronization: positive is 1, negative is 3
    DoubleBuffer<unsigned char> m_nonInvertedSyncSymbolBuffer; //!< same but resetting to positive sync

    static const int m_zeroCrossingCorrectionProfile2400[11];
    static const int m_zeroCrossingCorrectionProfile4800[11];
    static const int m_zeroCrossingCorrectionProfile9600[11];
};

} // namespace DSDcc

#endif /* DSD_SYMBOL_H_ */
