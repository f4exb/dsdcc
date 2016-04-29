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

#ifndef DSD_DVSERIAL_H_
#define DSD_DVSERIAL_H_

#include <string>

#include "dvcontroller.h"

namespace DSDcc
{

class DSDDecoder;

class DVController
{
public:
    DVController(std::string& dvDevice, DSDDecoder& dsdDecoder);
    ~DVController();

    bool processDVSerial(); //!< returns true if audio is available

    const short *getAudio() const { return m_upsampledAudioSamples; }
    int getNbAudioSamples() const { return m_nbAudioSamples; }

private:
    SerialDV::DVController m_dvController;
    DSDDecoder& m_dsdDecoder;
    const char *m_mbeFrame;
    unsigned char m_mbeBytes[SerialDV::VOICE_FRAME_LENGTH_BYTES];
    short m_dvAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE];
    short m_upsampledAudioSamples[SerialDV::MBE_AUDIO_BLOCK_SIZE*6];
    int m_nbAudioSamples;
};

} // namespace DSDcc

#endif /* DSD_DVSERIAL_H_ */
