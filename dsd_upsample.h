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

#ifndef DSD_UPSAMPLE_H_
#define DSD_UPSAMPLE_H_

namespace DSDcc
{

class DSDUpsampler
{
public:
    DSDUpsampler();
    ~DSDUpsampler();

    void upsampleOne(int upsampling, short inValue, short *outValues);
    void upsample(int upsampling, short *in, short *out, int nbSamplesIn);

private:
    short m_upsamplerLastValue;
};

} // namespace DSDcc

#endif /* DSD_UPSAMPLE_H_ */
