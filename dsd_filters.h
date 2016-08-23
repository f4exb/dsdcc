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

}

#endif /* DSDCC_DSD_FILTERS_H_ */
