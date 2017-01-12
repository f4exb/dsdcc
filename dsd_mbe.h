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

#ifndef DSDCC_DSD_MBE_H_
#define DSDCC_DSD_MBE_H_

#include "dsd_filters.h"

namespace DSDcc
{

class DSDDecoder;
struct DSDmbelibParms;

class DSDMBEDecoder
{
public:
    DSDMBEDecoder(DSDDecoder *dsdDecoder);
    ~DSDMBEDecoder();

    void initMbeParms();
    void processFrame(char imbe_fr[8][23], char ambe_fr[4][24], char imbe7100_fr[7][24]);
    void processData(char imbe_data[88], char ambe_data[49]);

    short *getAudio(int& nbSamples)
    {
        nbSamples = m_audio_out_nb_samples;
        return m_audio_out_buf;
    }

    void resetAudio()
    {
        m_audio_out_nb_samples = 0;
        m_audio_out_buf_p = m_audio_out_buf;
    }

    void setAudioGain(float aout_gain) { m_aout_gain = aout_gain; }
    void setAutoGain(bool auto_gain) { m_auto_gain = auto_gain; }
    void setStereo(bool stereo) { m_stereo = stereo; }
    void setChannels(unsigned char channels) { m_channels = channels % 4; }
    void setUpsamplingFactor(int upsample) { m_upsample = upsample; }
    int getUpsamplingFactor() const { return m_upsample; }

private:
    void processAudio();
    void upsample(int upsampling, float invalue);

    DSDDecoder *m_dsdDecoder;
    char imbe_d[88];
    char ambe_d[49];
    float m_upsamplerLastValue;

    DSDmbelibParms *m_mbelibParms;
    int m_errs;
    int m_errs2;
    char m_err_str[64];

    float m_audio_out_temp_buf[160];   //!< output of decoder
    float *m_audio_out_temp_buf_p;

    float m_audio_out_float_buf[1120]; //!< output of upsampler - 1 frame of 160 samples upampled up to 7 times
    float *m_audio_out_float_buf_p;

    float m_aout_max_buf[200];
    float *m_aout_max_buf_p;
    int m_aout_max_buf_idx;

    short m_audio_out_buf[2*48000];    //!< final result - 1s of L+R S16LE samples
    short *m_audio_out_buf_p;
    int   m_audio_out_nb_samples;
    int   m_audio_out_buf_size;
    int   m_audio_out_idx;
    int   m_audio_out_idx2;

    float m_aout_gain;
    bool m_auto_gain;
    int m_upsample;            //!< upsampling factor
    bool m_stereo;             //!< double each audio sample to produce L+R channels
    unsigned char m_channels;  //!< when in stereo output to none (0) or only left (1), right (2) or both (3) channels

    DSDMBEAudioInterpolatorFilter m_upsamplingFilter;
};

}

#endif /* DSDCC_DSD_MBE_H_ */
