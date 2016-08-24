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

}

#endif /* DSDCC_DSD_FILTERS_H_ */
