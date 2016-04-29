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

#include "dsd_decoder.h"
#include "dsd_dvserial.h"

namespace DSDcc
{

DVController::DVController(std::string& dvDevice, DSDDecoder& dsdDecoder) :
    m_dvController(dvDevice),
    m_dsdDecoder(dsdDecoder),
    m_nbAudioSamples(0)
{
    m_mbeFrame = m_dsdDecoder.getMbe();
}

DVController::~DVController()
{
}

bool DVController::processDVSerial()
{
    if (m_dsdDecoder.mbeReady())
    {
        DSDcc::DSDSymbol::compressBits(m_mbeFrame, m_mbeBytes, SerialDV::VOICE_FRAME_LENGTH_BYTES);

        if (m_dvController.decode(m_dvAudioSamples, m_mbeBytes))
        {
            if (m_dsdDecoder.upsampling())
            {
                // TODO: upsampler
                m_nbAudioSamples = SerialDV::MBE_AUDIO_BLOCK_SIZE * 6;
            }
            else
            {
                memcpy((void *) m_upsampledAudioSamples, (const void *) m_dvAudioSamples, SerialDV::MBE_AUDIO_BLOCK_BYTES);
                m_nbAudioSamples = SerialDV::MBE_AUDIO_BLOCK_SIZE;
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace DSDcc
