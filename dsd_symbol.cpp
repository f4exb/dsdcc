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

#include <stdlib.h>
#include <assert.h>

#include "dsd_symbol.h"
#include "dsd_decoder.h"

namespace DSDcc
{

DSDSymbol::DSDSymbol(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbol(0)
{
    resetSymbol();
    m_jitter = -1;
    m_symCount1 = 0;
}

DSDSymbol::~DSDSymbol()
{
}

void DSDSymbol::noCarrier()
{
    m_jitter = -1;
}

void DSDSymbol::resetFrameSync()
{
    m_symCount1 = 0;
}

void DSDSymbol::resetSymbol()
{
    m_sampleIndex = 0;
    m_sum = 0;
    m_count = 0;
}

bool DSDSymbol::pushSample(short sample, int have_sync)
{
    short inSample = sample;
    // timing control
    if ((m_sampleIndex == 0) && (have_sync == 0))
    {
        if (m_dsdDecoder->m_state.samplesPerSymbol == 20)
        {
            if ((m_jitter >= 7) && (m_jitter <= 10))
            {
                m_sampleIndex--;
            }
            else if ((m_jitter >= 11) && (m_jitter <= 14))
            {
                m_sampleIndex++;
            }
        }
        else if (m_dsdDecoder->m_state.rf_mod == 1) // QPSK
        {
            if ((m_jitter >= 0)
             && (m_jitter < m_dsdDecoder->m_state.symbolCenter))
            {
                m_sampleIndex++;          // fall back
            }
            else if ((m_jitter > m_dsdDecoder->m_state.symbolCenter)
                  && (m_jitter < 10))
            {
                m_sampleIndex--;          // catch up
            }
        }
        else if (m_dsdDecoder->m_state.rf_mod == 2) // GFSK
        {
            if ((m_jitter >= m_dsdDecoder->m_state.symbolCenter - 1)
             && (m_jitter <= m_dsdDecoder->m_state.symbolCenter))
            {
                m_sampleIndex--;
            }
            else if ((m_jitter >= m_dsdDecoder->m_state.symbolCenter + 1)
                  && (m_jitter <= m_dsdDecoder->m_state.symbolCenter + 2))
            {
                m_sampleIndex++;
            }
        }
        else if (m_dsdDecoder->m_state.rf_mod == 0) // C4FM
        {
            if ((m_jitter > 0)
             && (m_jitter <= m_dsdDecoder->m_state.symbolCenter))
            {
                m_sampleIndex--;          // catch up
            }
            else if ((m_jitter > m_dsdDecoder->m_state.symbolCenter)
                  && (m_jitter < m_dsdDecoder->m_state.samplesPerSymbol))
            {
                m_sampleIndex++;          // fall back
            }
        }

        m_jitter = -1;
    }

    // process sample
    if (m_dsdDecoder->m_opts.use_cosine_filter)
    {
        if (m_dsdDecoder->m_state.lastsynctype >= 10 && m_dsdDecoder->m_state.lastsynctype <= 13)              // all DMR
        {
            sample = m_dsdFilters.dmr_filter(sample);  // 12.5 kHz for DMR
        }
        else if (m_dsdDecoder->m_state.lastsynctype == 8  || m_dsdDecoder->m_state.lastsynctype == 9       // +/-NXDN
              || (m_dsdDecoder->m_state.lastsynctype == 20 || m_dsdDecoder->m_state.lastsynctype == 21))   // +DPMR FS1,4
        {
            if (m_dsdDecoder->m_state.samplesPerSymbol == 20) {
                sample = m_dsdFilters.nxdn_filter(sample); // 6.25 kHz for NXDN48 / DPMR
            } else {
                sample = m_dsdFilters.dmr_filter(sample);  // 12.5 kHz for NXDN96
            }
        }
    }

    if ((sample > m_dsdDecoder->m_state.max) && (have_sync == 1) && (m_dsdDecoder->m_state.rf_mod == 0))
    {
        sample = m_dsdDecoder->m_state.max;
    }
    else if ((sample < m_dsdDecoder->m_state.min) && (have_sync == 1)
            && (m_dsdDecoder->m_state.rf_mod == 0))
    {
        sample = m_dsdDecoder->m_state.min;
    }

    if (sample > m_dsdDecoder->m_state.center)
    {
        if (m_dsdDecoder->m_state.lastsample < m_dsdDecoder->m_state.center)
        {
            m_dsdDecoder->m_state.numflips += 1;
        }
        if (sample > (m_dsdDecoder->m_state.maxref * 1.25))
        {
            if (m_dsdDecoder->m_state.lastsample < (m_dsdDecoder->m_state.maxref * 1.25))
            {
                m_dsdDecoder->m_state.numflips += 1;
            }
            if ((m_jitter < 0) && (m_dsdDecoder->m_state.rf_mod == 1))
            {               // first spike out of place
                m_jitter = m_sampleIndex;
            }
            if ((m_dsdDecoder->m_opts.symboltiming == 1) && (have_sync == 0)
             && (m_dsdDecoder->m_state.lastsynctype != -1))
            {
                m_dsdDecoder->getLogger().log("O");
            }
        }
        else
        {
            if ((m_dsdDecoder->m_opts.symboltiming == 1) && (have_sync == 0)
             && (m_dsdDecoder->m_state.lastsynctype != -1))
            {
                m_dsdDecoder->getLogger().log("+");
            }
            if ((m_jitter < 0)
             && (m_dsdDecoder->m_state.lastsample < m_dsdDecoder->m_state.center)
             && (m_dsdDecoder->m_state.rf_mod != 1))
            {               // first transition edge
                m_jitter = m_sampleIndex;
            }
        }
    }
    else
    {                       // sample < 0
        if (m_dsdDecoder->m_state.lastsample > m_dsdDecoder->m_state.center)
        {
            m_dsdDecoder->m_state.numflips += 1;
        }
        if (sample < (m_dsdDecoder->m_state.minref * 1.25))
        {
            if (m_dsdDecoder->m_state.lastsample > (m_dsdDecoder->m_state.minref * 1.25))
            {
                m_dsdDecoder->m_state.numflips += 1;
            }
            if ((m_jitter < 0) && (m_dsdDecoder->m_state.rf_mod == 1))
            {               // first spike out of place
                m_jitter = m_sampleIndex;
            }
            if ((m_dsdDecoder->m_opts.symboltiming == 1) && (have_sync == 0)
             && (m_dsdDecoder->m_state.lastsynctype != -1))
            {
                m_dsdDecoder->getLogger().log("X");
            }
        }
        else
        {
            if ((m_dsdDecoder->m_opts.symboltiming == 1) && (have_sync == 0)
             && (m_dsdDecoder->m_state.lastsynctype != -1))
            {
                m_dsdDecoder->getLogger().log("-");
            }
            if ((m_jitter < 0)
             && (m_dsdDecoder->m_state.lastsample > m_dsdDecoder->m_state.center)
             && (m_dsdDecoder->m_state.rf_mod != 1))
            {               // first transition edge
                m_jitter = m_sampleIndex;
            }
        }
    }
    if (m_dsdDecoder->m_state.samplesPerSymbol == 20)
    {
        if ((m_sampleIndex >= 9) && (m_sampleIndex <= 11))
        {
            m_sum += sample;
            m_count++;
        }
    }
    if (m_dsdDecoder->m_state.samplesPerSymbol == 5)
    {
        if (m_sampleIndex == 2)
        {
            m_sum += sample;
            m_count++;
        }
    }
    else
    {
        // gr-dsd:
//        if (((m_sampleIndex >= m_dsdDecoder->m_state.symbolCenter - 1)
//          && (m_sampleIndex <= m_dsdDecoder->m_state.symbolCenter + 2)
//          && (m_dsdDecoder->m_state.rf_mod == 0))
//        || (((m_sampleIndex == m_dsdDecoder->m_state.symbolCenter)
//          || (m_sampleIndex == m_dsdDecoder->m_state.symbolCenter + 1))
//          && (m_dsdDecoder->m_state.rf_mod != 0)))
//        {
//            m_sum += sample;
//            m_count++;
//        }
        if (m_dsdDecoder->m_state.rf_mod == 0)
        {
            // 0: C4FM modulation
            if ((m_sampleIndex >= m_dsdDecoder->m_state.symbolCenter - 1)
             && (m_sampleIndex <= m_dsdDecoder->m_state.symbolCenter + 2))
            {
                m_sum += sample;
                m_count++;
            }
        }
        else
        {
            // 1: QPSK modulation
            // 2: GFSK modulation
            // Note: this has been changed to use an additional symbol to the left
            // On the p25_raw_unencrypted.flac it is evident that the timing
            // comes one sample too late.
            // This change makes a significant improvement in the BER, at least for
            // this file.
            //if ((i == state->symbolCenter) || (i == state->symbolCenter + 1))
            if ((m_sampleIndex == m_dsdDecoder->m_state.symbolCenter - 1)
             || (m_sampleIndex == m_dsdDecoder->m_state.symbolCenter + 1))
            {
                m_sum += sample;
                m_count++;
            }
        }
    }

    m_dsdDecoder->m_state.lastsample = sample;

    if (m_sampleIndex == m_dsdDecoder->m_state.samplesPerSymbol - 1) // conclusion
    {
        m_symbol = m_sum / m_count;
        if ((m_dsdDecoder->m_opts.symboltiming == 1) && (have_sync == 0)
                && (m_dsdDecoder->m_state.lastsynctype != -1))
        {
            if (m_jitter >= 0)
            {
                m_dsdDecoder->getLogger().log(" %i\n", m_jitter);
            }
            else
            {
                m_dsdDecoder->getLogger().log("\n");
            }
        }

        m_dsdDecoder->m_state.symbolcnt++;

        // Symbol debugging
//        if ((m_dsdDecoder->m_state.symbolcnt > 1160) && (m_dsdDecoder->m_state.symbolcnt < 1200))  { // sampling
//            m_dsdDecoder->getLogger().log("DSDSymbol::pushSample: symbol %d (%d:%d) in:%d\n", m_dsdDecoder->m_state.symbolcnt, m_symbol, sample, inSample);
//        }

        resetSymbol();

        // moved here what was done at symbol retrieval in the decoder

        if (m_symCount1 < 23)
        {
            m_symCount1++;
        }
        else
        {
            if (m_dsdDecoder->m_state.numflips > m_dsdDecoder->m_opts.mod_threshold)
            {
                if (m_dsdDecoder->m_opts.mod_qpsk == 1)
                {
                    m_dsdDecoder->m_state.rf_mod = 1;
                }
            }
            else if (m_dsdDecoder->m_state.numflips > 18)
            {
                if (m_dsdDecoder->m_opts.mod_gfsk == 1)
                {
                    m_dsdDecoder->m_state.rf_mod = 2;
                }
            }
            else
            {
                if (m_dsdDecoder->m_opts.mod_c4fm == 1)
                {
                    m_dsdDecoder->m_state.rf_mod = 0;
                }
            }

            m_dsdDecoder->m_state.numflips = 0;
            m_symCount1 = 0;
        }

        return true; // new symbol available
    }
    else
    {
        m_sampleIndex++; // wait for next sample
        return false;
    }
}

int DSDSymbol::get_dibit_and_analog_signal(int* out_analog_signal)
{
    int symbol;
    int dibit;

    m_dsdDecoder->m_state.numflips = 0;
    symbol = m_symbol;
    m_dsdDecoder->m_state.sbuf[m_dsdDecoder->m_state.sidx] = symbol;

    if (out_analog_signal != 0) {
        *out_analog_signal = symbol;
    }

    use_symbol(symbol);

    dibit = digitize(symbol);

    return dibit;
}

void DSDSymbol::use_symbol(int symbol)
{
    int i;
    int sbuf2[128];
    int lmin, lmax, lsum;

    for (i = 0; i < m_dsdDecoder->m_opts.ssize; i++)
    {
        sbuf2[i] = m_dsdDecoder->m_state.sbuf[i];
    }

    qsort(sbuf2, m_dsdDecoder->m_opts.ssize, sizeof(int), m_dsdDecoder->comp);

    // continuous update of min/max in rf_mod=1 (QPSK) mode
    // in c4fm min/max must only be updated during sync
    if (m_dsdDecoder->m_state.rf_mod == 1)
    {
        lmin = (sbuf2[0] + sbuf2[1]) / 2;
        lmax = (sbuf2[(m_dsdDecoder->m_opts.ssize - 1)] + sbuf2[(m_dsdDecoder->m_opts.ssize - 2)]) / 2;
        m_dsdDecoder->m_state.minbuf[m_dsdDecoder->m_state.midx] = lmin;
        m_dsdDecoder->m_state.maxbuf[m_dsdDecoder->m_state.midx] = lmax;
        if (m_dsdDecoder->m_state.midx == (m_dsdDecoder->m_opts.msize - 1))
        {
            m_dsdDecoder->m_state.midx = 0;
        }
        else
        {
            m_dsdDecoder->m_state.midx++;
        }
        lsum = 0;
        for (i = 0; i < m_dsdDecoder->m_opts.msize; i++)
        {
            lsum += m_dsdDecoder->m_state.minbuf[i];
        }
        m_dsdDecoder->m_state.min = lsum / m_dsdDecoder->m_opts.msize;
        lsum = 0;
        for (i = 0; i < m_dsdDecoder->m_opts.msize; i++)
        {
            lsum += m_dsdDecoder->m_state.maxbuf[i];
        }
        m_dsdDecoder->m_state.max = lsum / m_dsdDecoder->m_opts.msize;
        m_dsdDecoder->m_state.center = ((m_dsdDecoder->m_state.max) + (m_dsdDecoder->m_state.min)) / 2;
        m_dsdDecoder->m_state.umid = (((m_dsdDecoder->m_state.max) - m_dsdDecoder->m_state.center) * 5 / 8) + m_dsdDecoder->m_state.center;
        m_dsdDecoder->m_state.lmid = (((m_dsdDecoder->m_state.min) - m_dsdDecoder->m_state.center) * 5 / 8) + m_dsdDecoder->m_state.center;
        m_dsdDecoder->m_state.maxref = (int) ((m_dsdDecoder->m_state.max) * 0.80F);
        m_dsdDecoder->m_state.minref = (int) ((m_dsdDecoder->m_state.min) * 0.80F);
    }
    else
    {
        m_dsdDecoder->m_state.maxref = m_dsdDecoder->m_state.max;
        m_dsdDecoder->m_state.minref = m_dsdDecoder->m_state.min;
    }

    // Increase sidx
    if (m_dsdDecoder->m_state.sidx == (m_dsdDecoder->m_opts.ssize - 1))
    {
        m_dsdDecoder->m_state.sidx = 0;

        if (m_dsdDecoder->m_opts.datascope == 1)
        {
            print_datascope(sbuf2);
        }
    }
    else
    {
        m_dsdDecoder->m_state.sidx++;
    }

    if (m_dsdDecoder->m_state.dibit_buf_p > m_dsdDecoder->m_state.dibit_buf + 900000)
    {
        m_dsdDecoder->m_state.dibit_buf_p = m_dsdDecoder->m_state.dibit_buf + 200;
    }
}

int DSDSymbol::digitize(int symbol)
{
    // determine dibit state
    if ((m_dsdDecoder->m_state.synctype == 6) || (m_dsdDecoder->m_state.synctype == 14)
            || (m_dsdDecoder->m_state.synctype == 18))
    {
        //  6 +D-STAR
        // 14 +ProVoice
        // 18 +D-STAR_HD

        if (symbol > m_dsdDecoder->m_state.center)
        {
            *m_dsdDecoder->m_state.dibit_buf_p = 1;
            m_dsdDecoder->m_state.dibit_buf_p++;
            return (0);               // +1
        }
        else
        {
            *m_dsdDecoder->m_state.dibit_buf_p = 3;
            m_dsdDecoder->m_state.dibit_buf_p++;
            return (1);               // +3
        }
    }
    else if ((m_dsdDecoder->m_state.synctype == 7) || (m_dsdDecoder->m_state.synctype == 15)
            || (m_dsdDecoder->m_state.synctype == 19))
    {
        //  7 -D-STAR
        // 15 -ProVoice
        // 19 -D-STAR_HD

        if (symbol > m_dsdDecoder->m_state.center)
        {
            *m_dsdDecoder->m_state.dibit_buf_p = 1;
            m_dsdDecoder->m_state.dibit_buf_p++;
            return (1);               // +3
        }
        else
        {
            *m_dsdDecoder->m_state.dibit_buf_p = 3;
            m_dsdDecoder->m_state.dibit_buf_p++;
            return (0);               // +1
        }
    }
    else if ((m_dsdDecoder->m_state.synctype == 1) || (m_dsdDecoder->m_state.synctype == 3)
            || (m_dsdDecoder->m_state.synctype == 5) || (m_dsdDecoder->m_state.synctype == 9)
            || (m_dsdDecoder->m_state.synctype == 11) || (m_dsdDecoder->m_state.synctype == 13)
            || (m_dsdDecoder->m_state.synctype == 17))
    {
        //  1 -P25p1
        //  3 -X2-TDMA (inverted signal voice frame)
        //  5 -X2-TDMA (inverted signal data frame)
        //  9 -NXDN (inverted voice frame)
        // 11 -DMR (inverted signal voice frame)
        // 13 -DMR (inverted signal data frame)
        // 17 -NXDN (inverted data frame)

        int valid;
        int dibit;

        valid = 0;

        if (m_dsdDecoder->m_state.synctype == 1)
        {
            // Use the P25 heuristics if available
            valid = DSDP25Heuristics::estimate_symbol(m_dsdDecoder->m_state.rf_mod, &(m_dsdDecoder->m_state.inv_p25_heuristics),
                    m_dsdDecoder->m_state.last_dibit, symbol, &dibit);
        }

        if (valid == 0)
        {
            // Revert to the original approach: choose the symbol according to the regions delimited
            // by center, umid and lmid
            if (symbol > m_dsdDecoder->m_state.center)
            {
                if (symbol > m_dsdDecoder->m_state.umid)
                {
                    dibit = 3;               // -3
                }
                else
                {
                    dibit = 2;               // -1
                }
            }
            else
            {
                if (symbol < m_dsdDecoder->m_state.lmid)
                {
                    dibit = 1;               // +3
                }
                else
                {
                    dibit = 0;               // +2
                }
            }
        }

        m_dsdDecoder->m_state.last_dibit = dibit;

        // store non-inverted values in dibit_buf
        *m_dsdDecoder->m_state.dibit_buf_p = invert_dibit(dibit);
        m_dsdDecoder->m_state.dibit_buf_p++;
        return dibit;
    }
    else
    {
        //  0 +P25p1
        //  2 +X2-TDMA (non inverted signal data frame)
        //  4 +X2-TDMA (non inverted signal voice frame)
        //  8 +NXDN (non inverted voice frame)
        // 10 +DMR (non inverted signal data frame)
        // 12 +DMR (non inverted signal voice frame)
        // 16 +NXDN (non inverted data frame)

        int valid;
        int dibit;

        valid = 0;

        if (m_dsdDecoder->m_state.synctype == 0)
        {
            // Use the P25 heuristics if available
            valid = DSDP25Heuristics::estimate_symbol(m_dsdDecoder->m_state.rf_mod, &(m_dsdDecoder->m_state.p25_heuristics),
                    m_dsdDecoder->m_state.last_dibit, symbol, &dibit);
        }

        if (valid == 0)
        {
            // Revert to the original approach: choose the symbol according to the regions delimited
            // by center, umid and lmid
            if (symbol > m_dsdDecoder->m_state.center)
            {
                if (symbol > m_dsdDecoder->m_state.umid)
                {
                    dibit = 1;               // +3
                }
                else
                {
                    dibit = 0;               // +1
                }
            }
            else
            {
                if (symbol < m_dsdDecoder->m_state.lmid)
                {
                    dibit = 3;               // -3
                }
                else
                {
                    dibit = 2;               // -1
                }
            }
        }

        m_dsdDecoder->m_state.last_dibit = dibit;

        *m_dsdDecoder->m_state.dibit_buf_p = dibit;
        m_dsdDecoder->m_state.dibit_buf_p++;
        return dibit;
    }
}

int DSDSymbol::invert_dibit(int dibit)
{
    switch (dibit)
    {
    case 0:
        return 2;
    case 1:
        return 3;
    case 2:
        return 0;
    case 3:
        return 1;
    }

    // Error, shouldn't be here
    assert(0);
    return -1;
}

void DSDSymbol::print_datascope(int* sbuf2)
{
    int i, j, o;
    char modulation[8];
    int spectrum[64];

    if (m_dsdDecoder->m_state.rf_mod == 0)
    {
        sprintf(modulation, "C4FM");
    }
    else if (m_dsdDecoder->m_state.rf_mod == 1)
    {
        sprintf(modulation, "QPSK");
    }
    else if (m_dsdDecoder->m_state.rf_mod == 2)
    {
        sprintf(modulation, "GFSK");
    }

    for (i = 0; i < 64; i++)
    {
        spectrum[i] = 0;
    }
    for (i = 0; i < m_dsdDecoder->m_opts.ssize; i++)
    {
        o = (sbuf2[i] + 32768) / 1024;
        spectrum[o]++;
    }
    if (m_dsdDecoder->m_state.symbolcnt > (4800 / m_dsdDecoder->m_opts.scoperate))
    {
        m_dsdDecoder->m_state.symbolcnt = 0;
        m_dsdDecoder->getLogger().log("\n");
        m_dsdDecoder->getLogger().log(
                "Demod mode:     %s                Nac:                     %4X\n",
                modulation, m_dsdDecoder->m_state.nac);
        m_dsdDecoder->getLogger().log("Frame Type:    %s        Talkgroup:            %7i\n",
                m_dsdDecoder->m_state.ftype, m_dsdDecoder->m_state.lasttg);
        m_dsdDecoder->getLogger().log("Frame Subtype: %s       Source:          %12i\n",
                m_dsdDecoder->m_state.fsubtype, m_dsdDecoder->m_state.lastsrc);
        m_dsdDecoder->getLogger().log("TDMA activity:  %s %s     Voice errors: %s\n",
                m_dsdDecoder->m_state.slot0light, m_dsdDecoder->m_state.slot1light, m_dsdDecoder->m_state.err_str);
        m_dsdDecoder->getLogger().log(
                "+----------------------------------------------------------------+\n");
        for (i = 0; i < 10; i++)
        {
            m_dsdDecoder->getLogger().log("|");
            for (j = 0; j < 64; j++)
            {
                if (i == 0)
                {
                    if ((j == ((m_dsdDecoder->m_state.min) + 32768) / 1024)
                            || (j == ((m_dsdDecoder->m_state.max) + 32768) / 1024))
                    {
                        m_dsdDecoder->getLogger().log("#");
                    }
                    else if ((j == ((m_dsdDecoder->m_state.lmid) + 32768) / 1024)
                            || (j == ((m_dsdDecoder->m_state.umid) + 32768) / 1024))
                    {
                        m_dsdDecoder->getLogger().log("^");
                    }
                    else if (j == (m_dsdDecoder->m_state.center + 32768) / 1024)
                    {
                        m_dsdDecoder->getLogger().log("!");
                    }
                    else
                    {
                        if (j == 32)
                        {
                            m_dsdDecoder->getLogger().log("|");
                        }
                        else
                        {
                            m_dsdDecoder->getLogger().log(" ");
                        }
                    }
                }
                else
                {
                    if (spectrum[j] > 9 - i)
                    {
                        m_dsdDecoder->getLogger().log("*");
                    }
                    else
                    {
                        if (j == 32)
                        {
                            m_dsdDecoder->getLogger().log("|");
                        }
                        else
                        {
                            m_dsdDecoder->getLogger().log(" ");
                        }
                    }
                }
            }
            m_dsdDecoder->getLogger().log("|\n");
        }
        m_dsdDecoder->getLogger().log(
                "+----------------------------------------------------------------+\n");
    }
}

int DSDSymbol::getDibit()
{
    return get_dibit_and_analog_signal(0);
}

} // namespace DSDcc
