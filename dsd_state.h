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

extern "C" {
#include <mbelib.h>
}

#include "p25p1_heuristics.h"

namespace DSDcc
{

class DSDState
{
public:
    DSDState();
    ~DSDState();

    int *dibit_buf;
    int *dibit_buf_p;
    int repeat;
    short *audio_out_buf;          //!< final result
    short *audio_out_buf_p;
    int   audio_out_nb_samples;
    int   audio_out_buf_size;
    float *audio_out_float_buf;    //!< output of upsampler
    float *audio_out_float_buf_p;
    float audio_out_temp_buf[160]; //!< output of decoder
    float *audio_out_temp_buf_p;
    int audio_out_idx;
    int audio_out_idx2;
    int center;
    int synctype;
    int min;
    int max;
    int lmid;
    int umid;
    int lastsample;
    int sbuf[128];
    int sidx;
    int maxbuf[1024];
    int minbuf[1024];
    int midx;
    char err_str[64];
    char fsubtype[16];
    char ftype[16];
    int symbolcnt;
    int rf_mod;
    int numflips;
    int lastsynctype;
    int lastp25type;
    int offset;
    int carrier;
    char tg[25][16];
    int tgcount;
    int lasttg;
    int lastsrc;
    int nac;
    int errs;
    int errs2;
    int mbe_file_type;
    int optind;
    int numtdulc;
    int firstframe;
    char slot0light[8];
    char slot1light[8];
    unsigned char ccnum;
    float aout_gain;
    float aout_max_buf[200];
    float *aout_max_buf_p;
    int aout_max_buf_idx;
    int samplesPerSymbol;
    int symbolCenter;
    char algid[9];
    char keyid[17];
    int currentslot;
    mbe_parms *cur_mp;
    mbe_parms *prev_mp;
    mbe_parms *prev_mp_enhanced;
    int p25kid;
    int last_dibit;                                     //!< Last dibit read
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
