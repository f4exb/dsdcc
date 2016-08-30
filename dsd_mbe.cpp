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

#include <string.h>
#include <math.h>
#include "dsd_mbe.h"
#include "dsd_decoder.h"

namespace DSDcc
{

DSDMBEDecoder::DSDMBEDecoder(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_upsamplerLastValue(0.0f)
{
    m_cur_mp = (mbe_parms *) malloc(sizeof(mbe_parms));
    m_prev_mp = (mbe_parms *) malloc(sizeof(mbe_parms));
    m_prev_mp_enhanced = (mbe_parms *) malloc(sizeof(mbe_parms));

    m_audio_out_temp_buf_p = m_audio_out_temp_buf;
    memset(m_audio_out_float_buf, 0, sizeof(float) * 1120);
    m_audio_out_float_buf_p = m_audio_out_float_buf;
    memset(m_aout_max_buf, 0, sizeof(float) * 200);
    m_aout_max_buf_p = m_aout_max_buf;
    m_aout_max_buf_idx = 0;

    memset(m_audio_out_buf, 0, sizeof(short) * 2 * 48000);
    m_audio_out_buf_p = m_audio_out_buf;
    m_audio_out_nb_samples = 0;
    m_audio_out_buf_size = 48000; // given in number of unique samples
    m_audio_out_idx = 0;
    m_audio_out_idx2 = 0;

	initMbeParms();
}

DSDMBEDecoder::~DSDMBEDecoder()
{
    free(m_prev_mp_enhanced);
    free(m_prev_mp);
    free(m_cur_mp);
}

void DSDMBEDecoder::initMbeParms()
{
	mbe_initMbeParms(m_cur_mp, m_prev_mp, m_prev_mp_enhanced);
	m_errs = 0;
	m_errs2 = 0;
	m_err_str[0] = 0;
}

void DSDMBEDecoder::processFrame(char imbe_fr[8][23], char ambe_fr[4][24], char imbe7100_fr[7][24])
{
    if (!m_dsdDecoder->m_mbelibEnable) {
        return;
    }
#ifdef DSD_USE_MBELIB
    memset((void *) imbe_d, 0, 88);

    if (m_dsdDecoder->m_mbeRate == DSDDecoder::DSDMBERate7200x4400)
    {
        mbe_processImbe7200x4400Framef(m_audio_out_temp_buf, &m_errs,
                &m_errs2, m_err_str, imbe_fr, imbe_d, m_cur_mp,
                m_prev_mp, m_prev_mp_enhanced, m_dsdDecoder->m_opts.uvquality);
    }
    else if (m_dsdDecoder->m_mbeRate == DSDDecoder::DSDMBERate7100x4400)
    {
        mbe_processImbe7100x4400Framef(m_audio_out_temp_buf, &m_errs,
                &m_errs2, m_err_str, imbe7100_fr, imbe_d,
                m_cur_mp, m_prev_mp, m_prev_mp_enhanced,
                m_dsdDecoder->m_opts.uvquality);
    }
    else if (m_dsdDecoder->m_mbeRate == DSDDecoder::DSDMBERate3600x2400)
    {
        mbe_processAmbe3600x2400Framef(m_audio_out_temp_buf, &m_errs,
                &m_errs2, m_err_str, ambe_fr, ambe_d, m_cur_mp,
                m_prev_mp, m_prev_mp_enhanced, m_dsdDecoder->m_opts.uvquality);
    }
    else
    {
        mbe_processAmbe3600x2450Framef(m_audio_out_temp_buf, &m_errs,
                &m_errs2, m_err_str, ambe_fr, ambe_d, m_cur_mp,
                m_prev_mp, m_prev_mp_enhanced, m_dsdDecoder->m_opts.uvquality);
    }

    if (m_dsdDecoder->m_opts.errorbars == 1)
    {
        m_dsdDecoder->getLogger().log("%s", m_err_str);
    }

    processAudio();
#endif
}

void DSDMBEDecoder::processAudio()
{
    int i, n;
    float aout_abs, max, gainfactor, gaindelta, maxbuf;

    if (m_dsdDecoder->m_opts.audio_gain == (float) 0)
    {
        // detect max level
        max = 0;
        m_audio_out_temp_buf_p = m_audio_out_temp_buf;

        for (n = 0; n < 160; n++)
        {
            aout_abs = fabsf(*m_audio_out_temp_buf_p);

            if (aout_abs > max)
            {
                max = aout_abs;
            }

            m_audio_out_temp_buf_p++;
        }

        *m_aout_max_buf_p = max;
        m_aout_max_buf_p++;
        m_aout_max_buf_idx++;

        if (m_aout_max_buf_idx > 24)
        {
            m_aout_max_buf_idx = 0;
            m_aout_max_buf_p = m_aout_max_buf;
        }

        // lookup max history
        for (i = 0; i < 25; i++)
        {
            maxbuf = m_aout_max_buf[i];

            if (maxbuf > max)
            {
                max = maxbuf;
            }
        }

        // determine optimal gain level
        if (max > (float) 0)
        {
            gainfactor = ((float) 30000 / max);
        }
        else
        {
            gainfactor = (float) 50;
        }

        if (gainfactor < m_dsdDecoder->m_state.aout_gain)
        {
            m_dsdDecoder->m_state.aout_gain = gainfactor;
            gaindelta = (float) 0;
        }
        else
        {
            if (gainfactor > (float) 50)
            {
                gainfactor = (float) 50;
            }

            gaindelta = gainfactor - m_dsdDecoder->m_state.aout_gain;

            if (gaindelta > ((float) 0.05 * m_dsdDecoder->m_state.aout_gain))
            {
                gaindelta = ((float) 0.05 * m_dsdDecoder->m_state.aout_gain);
            }
        }

        gaindelta /= (float) 160;
    }
    else
    {
        gaindelta = (float) 0;
    }

    if (m_dsdDecoder->m_opts.audio_gain >= 0)
    {
        // adjust output gain
        m_audio_out_temp_buf_p = m_audio_out_temp_buf;

        for (n = 0; n < 160; n++)
        {
            *m_audio_out_temp_buf_p = (m_dsdDecoder->m_state.aout_gain
                    + ((float) n * gaindelta)) * (*m_audio_out_temp_buf_p);
            m_audio_out_temp_buf_p++;
        }

        m_dsdDecoder->m_state.aout_gain += ((float) 160 * gaindelta);
    }

    // copy audio data to output buffer and upsample if necessary
    m_audio_out_temp_buf_p = m_audio_out_temp_buf;

    if ((m_dsdDecoder->m_opts.upsample == 6) || (m_dsdDecoder->m_opts.upsample == 7)) // upsampling to 48k
    {
        int upsampling = m_dsdDecoder->m_opts.upsample;

        if (m_audio_out_nb_samples + (160*upsampling) >= m_audio_out_buf_size)
        {
            m_dsdDecoder->resetAudio();
        }

        m_audio_out_float_buf_p = m_audio_out_float_buf;

        for (n = 0; n < 160; n++)
        {
            upsample(upsampling, *m_audio_out_temp_buf_p);
            m_audio_out_temp_buf_p++;
            m_audio_out_float_buf_p += upsampling;
            m_audio_out_idx += upsampling;
            m_audio_out_idx2 += upsampling;
        }

        m_audio_out_float_buf_p = m_audio_out_float_buf;

        // copy to output (short) buffer
        for (n = 0; n < (160*upsampling); n++)
        {
            if (*m_audio_out_float_buf_p > (float) 32760)
            {
                *m_audio_out_float_buf_p = (float) 32760;
            }
            else if (*m_audio_out_float_buf_p < (float) -32760)
            {
                *m_audio_out_float_buf_p = (float) -32760;
            }

            *m_audio_out_buf_p = (short) *m_audio_out_float_buf_p;
            m_audio_out_buf_p++;

            if (m_dsdDecoder->m_opts.stereo) // produce second channel
            {
                *m_audio_out_buf_p = (short) *m_audio_out_float_buf_p;
                m_audio_out_buf_p++;
            }

            m_audio_out_nb_samples++;
            m_audio_out_float_buf_p++;
        }
    }
    else // leave at 8k
    {
        if (m_audio_out_nb_samples + 160 >= m_audio_out_buf_size)
        {
            m_dsdDecoder->resetAudio();
        }

        m_audio_out_float_buf_p = m_audio_out_float_buf;

        for (n = 0; n < 160; n++)
        {
            if (*m_audio_out_temp_buf_p > (float) 32760)
            {
                *m_audio_out_temp_buf_p = (float) 32760;
            }
            else if (*m_audio_out_temp_buf_p < (float) -32760)
            {
                *m_audio_out_temp_buf_p = (float) -32760;
            }

            *m_audio_out_buf_p = (short) *m_audio_out_temp_buf_p;
            m_audio_out_buf_p++;

            if (m_dsdDecoder->m_opts.stereo) // produce second channel
            {
                *m_audio_out_buf_p = (short) *m_audio_out_float_buf_p;
                m_audio_out_buf_p++;
            }

            m_audio_out_nb_samples++;
            m_audio_out_temp_buf_p++;
            m_audio_out_idx++;
            m_audio_out_idx2++;
        }
    }
}

void DSDMBEDecoder::upsample(int upsampling, float invalue)
{
    int i, j, sum;
    float *outbuf1, c, d;

    outbuf1 = m_audio_out_float_buf_p;
//    outbuf1--;
//    c = *outbuf1;
    c = m_upsamplerLastValue;
    d = invalue;
    // basic triangle interpolation
//    outbuf1++;
    if (upsampling == 6)
    {
        *outbuf1 = ((invalue * (float) 0.166) + (c * (float) 0.834));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.332) + (c * (float) 0.668));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.5) + (c * (float) 0.5));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.668) + (c * (float) 0.332));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.834) + (c * (float) 0.166));
        outbuf1++;
        *outbuf1 = d;
        m_upsamplerLastValue = d;
        outbuf1++;
    }
    else if (upsampling == 7)
    {
        *outbuf1 = ((invalue * (float) 0.142) + (c * (float) 0.857));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.286) + (c * (float) 0.714));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.429) + (c * (float) 0.571));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.571) + (c * (float) 0.429));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.714) + (c * (float) 0.286));
        outbuf1++;
        *outbuf1 = ((invalue * (float) 0.857) + (c * (float) 0.142));
        outbuf1++;
        *outbuf1 = d;
        m_upsamplerLastValue = d;
        outbuf1++;
    }
    else // no upsampling
    {
        outbuf1++;
        *outbuf1 = d;
        outbuf1++;
    }

    outbuf1 -= upsampling;

    // FIXME: this is all wrong! Now at least it does not corrupt audio_out_float_buf

//    if (m_dsdDecoder->m_state.audio_out_idx2 > 24)
//    {
//        // smoothing
//        outbuf1 -= 16;
//        for (j = 0; j < 4; j++)
//        {
//            for (i = 0; i < m_upsampling; i++)
//            {
//                sum = 0;
//                outbuf1 -= 2;
//                sum += *outbuf1;
//                outbuf1 += 2;
//                sum += *outbuf1;
//                outbuf1 += 2;
//                sum += *outbuf1;
//                outbuf1 -= 2;
//                *outbuf1 = (sum / (float) 3);
//                outbuf1++;
//            }
//            outbuf1 -= m_upsampling + 2;
//        }
//    }
}


}
