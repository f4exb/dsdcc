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

#ifndef DSDCC_DSD_STATE_H_
#define DSDCC_DSD_STATE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "p25p1_heuristics.h"
#include "export.h"

namespace DSDcc
{

class DSDCC_API DSDState
{
public:
    DSDState();
    ~DSDState();

    int repeat;
    int maxbuf[1024];
    int minbuf[1024];
    int midx;
    char fsubtype[16];
    char ftype[16];
    int symbolcnt;
    int lastp25type;
    int offset;
    int carrier;
    char tg[25][16];
    int tgcount;
    int lasttg;
    int lastsrc;
    int nac;
    int mbe_file_type;
    int optind;
    int numtdulc;
    int firstframe;
    char slot0light[27];
    char slot1light[27];
    unsigned char ccnum;
    char algid[9];
    char keyid[17];
    int currentslot;
    int p25kid;
    DSDP25Heuristics::P25Heuristics p25_heuristics;     //!< Heuristics state data for +P5 signals
    DSDP25Heuristics::P25Heuristics inv_p25_heuristics; //!< Heuristics state data for -P5 signals

    short *output_buffer;
    int output_offset;
    float *output_samples;
    int output_num_samples;
    int output_length;
    int output_finished;
};

} // namespace dsdcc

#endif /* DSDCC_DSD_STATE_H_ */
