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

#ifndef DSDCC_DSD_OPTS_H_
#define DSDCC_DSD_OPTS_H_

#include "export.h"

namespace DSDcc
{

class DSDCC_API DSDOpts
{
public:
    DSDOpts();
    ~DSDOpts();

    int onesymbol;
    int errorbars;
    int symboltiming;
    int verbose;
    unsigned char dmr_bp_key;
    int p25enc;
    int p25lc;
    int p25status;
    int p25tg;
    int scoperate;
    int playoffset;
    float audio_gain;
    int audio_out;
    int resume;
    int frame_dstar;
    int frame_x2tdma;
    int frame_p25p1;
    int frame_nxdn48;
    int frame_nxdn96;
    int frame_dmr;
    int frame_provoice;
    int frame_dpmr;
    int frame_ysf;
    int uvquality;
    int inverted_x2tdma;
    int delay;
    int use_cosine_filter;
    int unmute_encrypted_p25;
};

} // namespace dsdcc

#endif /* DSDCC_DSD_OPTS_H_ */
