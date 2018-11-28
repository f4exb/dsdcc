///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017 Edouard Griffiths, F4EXB.                                  //
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

#ifndef DSD_MBELIB_H_
#define DSD_MBELIB_H_

#include <stdlib.h>
#include "export.h"

extern "C" {
#include <mbelib.h>
}

namespace DSDcc
{

struct DSDCC_API DSDmbelibParms
{
    mbe_parms *m_cur_mp;
    mbe_parms *m_prev_mp;
    mbe_parms *m_prev_mp_enhanced;

    DSDmbelibParms()
    {
        m_cur_mp = (mbe_parms *) malloc(sizeof(mbe_parms));
        m_prev_mp = (mbe_parms *) malloc(sizeof(mbe_parms));
        m_prev_mp_enhanced = (mbe_parms *) malloc(sizeof(mbe_parms));
    }

    ~DSDmbelibParms()
    {
        free(m_prev_mp_enhanced);
        free(m_prev_mp);
        free(m_cur_mp);
    }
};

}

#endif /* DSD_MBELIB_H_ */
