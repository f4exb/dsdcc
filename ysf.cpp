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

#include "ysf.h"
#include "dsd_decoder.h"

namespace DSDcc
{

DSDYSF::DSDYSF(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbolIndex(0)
{
}

DSDYSF::~DSDYSF()
{
}

void DSDYSF::init()
{
    m_symbolIndex = 0;
}

void DSDYSF::process() // just pass the frames for now
{
    if (m_symbolIndex < 480 - 20)
    {
        m_symbolIndex++;
    }
    else
    {
        m_dsdDecoder->resetFrameSync(); // end
    }
}


} // namespace DSDcc
