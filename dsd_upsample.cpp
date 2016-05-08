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

#include "dsd_upsample.h"

namespace DSDcc
{

DSDUpsampler::DSDUpsampler() :
        m_upsamplerLastValue(0)
{
}

DSDUpsampler::~DSDUpsampler()
{
}

void DSDUpsampler::upsampleOne(int upsampling, short inValue, short *outValues)
{
    // copy to ints to gain precision
    int cur = (int) inValue;
    int prev = (int) m_upsamplerLastValue;

    if (upsampling == 6)
    {
        *outValues = (cur*1 + prev*5) / 6;
        outValues++;
        *outValues = (cur*2 + prev*4) / 6;
        outValues++;
        *outValues = (cur*3 + prev*3) / 6;
        outValues++;
        *outValues = (cur*4 + prev*2) / 6;
        outValues++;
        *outValues = (cur*5 + prev*1) / 6;
        outValues++;
        *outValues = inValue;
        m_upsamplerLastValue = inValue;
    }
    else if (upsampling == 7)
    {
        *outValues = (cur*1 + prev*6) / 7;
        outValues++;
        *outValues = (cur*2 + prev*5) / 7;
        outValues++;
        *outValues = (cur*3 + prev*4) / 7;
        outValues++;
        *outValues = (cur*4 + prev*3) / 7;
        outValues++;
        *outValues = (cur*5 + prev*2) / 7;
        outValues++;
        *outValues = (cur*6 + prev*1) / 7;
        outValues++;
        *outValues = inValue;
        m_upsamplerLastValue = inValue;
    }
}

void DSDUpsampler::upsample(int upsampling, short *in, short *out, int nbSamplesIn)
{
    for (int i = 0; i < nbSamplesIn; i++)
    {
        upsampleOne(upsampling, in[i], &out[upsampling*i]);
    }
}

} // namespace DSDcc


