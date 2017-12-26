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

#ifndef DSDCC_DSD_FILTERS_H_
#define DSDCC_DSD_FILTERS_H_

#define NZEROS 60
#define NXZEROS 134

#include "iirfilter.h"

namespace DSDcc
{

class DSDFilters
{
public:
    DSDFilters();
    ~DSDFilters();

    static const float ngain;
    static const float xcoeffs[];
    static const float nxgain;
    static const float nxcoeffs[];
    static const float dmrgain;
    static const float dmrcoeffs[];
    static const float dpmrgain;
    static const float dpmrcoeffs[];

    short dsd_input_filter(short sample, int mode);
    short dmr_filter(short sample);
    short nxdn_filter(short sample);

private:
    float xv[NZEROS+1];
    float nxv[NXZEROS+1];
};

/**
 * \Brief: This is a second order bandpass filter using recursive method. r is in range ]0..1[ the higher the steeper the filter.
 * inspired by:http://www.ece.umd.edu/~tretter/commlab/c6713slides/FSKSlides.pdf
 */
class DSDSecondOrderRecursiveFilter
{
public:
    DSDSecondOrderRecursiveFilter(float samplingFrequency, float centerFrequency, float r);
    ~DSDSecondOrderRecursiveFilter();

    void setFrequencies(float samplingFrequency, float centerFrequency);
    void setR(float r);
    short run(short sample);

private:
    void init();

    float m_r;
    float m_frequencyRatio;
    float m_v[3];
};

/**
 * This is a 2 pole lowpass Chebyshev (recursive) filter at fc=0.075 using coefficients found in table 20-1 of
 * http://www.analog.com/media/en/technical-documentation/dsp-book/dsp_book_Ch20.pdf
 *
 * At the interpolated sampling frequency of 48 kHz the -3 dB corner is at 48 * .075 = 3.6 kHz which is perfect for voice
 *
 * a0= 3.869430E-02
 * a1= 7.738860E-02 b1= 1.392667E+00
 * a2= 3.869430E-02 b2= -5.474446E-01
 *
 * given x[n] is the new input sample and y[n] the returned output sample:
 *
 * y[n] = a0*x[n] + a1*x[n] + a2*x[n] + b1*y[n-1] + b2*y[n-2]
 *
 * This one works directly with floats
 *
 */

class DSDMBEAudioInterpolatorFilter
{
public:
    DSDMBEAudioInterpolatorFilter();
    ~DSDMBEAudioInterpolatorFilter();

    void useHP(bool useHP) { m_useHP = useHP; }
    float run(const float& sample);

private:
    IIRFilter<float, 2> m_filterLP;
    IIRFilter<float, 2> m_filterHP;
    bool m_useHP;
    static const float m_lpa[3];
    static const float m_lpb[3];
    static const float m_hpa[3];
    static const float m_hpb[3];
};

}

#endif /* DSDCC_DSD_FILTERS_H_ */
