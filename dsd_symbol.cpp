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
    m_umid = 0;
    m_lmid = 0;
    m_nbFSKSymbols = 2;
    m_invertedFSK = false;
}

DSDSymbol::~DSDSymbol()
{
}

void DSDSymbol::noCarrier()
{
    m_jitter = -1;
    m_max = 15000;
    m_min = -15000;
    m_center = 0;
}

void DSDSymbol::resetFrameSync()
{
    m_symCount1 = 0;
    m_lmin = 0;
    m_lmax = 0;
    m_numflips = 0;
}

void DSDSymbol::resetSymbol()
{
    m_sampleIndex = 0;
    m_sum = 0;
    m_count = 0;
}

bool DSDSymbol::pushSample(short sample, bool have_sync)
{
    // short inSample = sample; // DEBUG

    // timing control
    if ((m_sampleIndex == 0) && (!have_sync))
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

    if (sample > m_center)
    {
        if (m_dsdDecoder->m_state.lastsample < m_center)
        {
            m_numflips += 1;
        }
        if (sample > (m_max * 1.25))
        {
            if (m_dsdDecoder->m_state.lastsample < (m_max * 1.25))
            {
                m_numflips += 1;
            }
            if ((m_dsdDecoder->m_opts.symboltiming == 1) && (!have_sync)
             && (m_dsdDecoder->m_state.lastsynctype != -1))
            {
                m_dsdDecoder->getLogger().log("O");
            }
        }
        else
        {
            if ((m_dsdDecoder->m_opts.symboltiming == 1) && (!have_sync)
             && (m_dsdDecoder->m_state.lastsynctype != -1))
            {
                m_dsdDecoder->getLogger().log("+");
            }
            if ((m_jitter < 0)
             && (m_dsdDecoder->m_state.lastsample < m_center))
            {               // first transition edge
                m_jitter = m_sampleIndex;
            }
        }
    }
    else
    {                       // sample < 0
        if (m_dsdDecoder->m_state.lastsample > m_center)
        {
            m_numflips += 1;
        }
        if (sample < (m_min * 1.25))
        {
            if (m_dsdDecoder->m_state.lastsample > (m_min * 1.25))
            {
                m_numflips += 1;
            }
            if ((m_dsdDecoder->m_opts.symboltiming == 1) && (!have_sync)
             && (m_dsdDecoder->m_state.lastsynctype != -1))
            {
                m_dsdDecoder->getLogger().log("X");
            }
        }
        else
        {
            if ((m_dsdDecoder->m_opts.symboltiming == 1) && (!have_sync)
             && (m_dsdDecoder->m_state.lastsynctype != -1))
            {
                m_dsdDecoder->getLogger().log("-");
            }
            if ((m_jitter < 0)
             && (m_dsdDecoder->m_state.lastsample > m_center))
            {               // first transition edge
                m_jitter = m_sampleIndex;
            }
        }
    }

    if (m_dsdDecoder->m_state.samplesPerSymbol == 5)
    {
        if (m_sampleIndex == m_dsdDecoder->m_state.symbolCenter)
        {
            m_sum += sample;
            m_count++;
        }
    }
    else
    {
        if ((m_sampleIndex >= m_dsdDecoder->m_state.symbolCenter - 1)
         && (m_sampleIndex <= m_dsdDecoder->m_state.symbolCenter + 1))
        {
            m_sum += sample;
            m_count++;
        }
    }

    m_dsdDecoder->m_state.lastsample = sample;

    if (m_sampleIndex == m_dsdDecoder->m_state.samplesPerSymbol - 1) // conclusion
    {
        m_symbol = m_sum / m_count;

        if ((m_dsdDecoder->m_opts.symboltiming == 1) && (!have_sync)
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

        // min/max calculation

        if (m_lidx < 23) {
            m_lidx++;
        } else {
            m_lidx = 0;
        }

        m_lbuf[m_lidx]    = m_symbol; // double buffering
        m_lbuf[m_lidx+32] = m_symbol;

        return true; // new symbol available
    }
    else
    {
        m_sampleIndex++; // wait for next sample
        return false;
    }
}

void DSDSymbol::snapSync(int nbSymbols)
{
    memcpy(m_lbuf2, &m_lbuf[32 + m_lidx - nbSymbols], nbSymbols * sizeof(int)); // copy to working buffer
    qsort(m_lbuf2, nbSymbols, sizeof(int), comp);

    m_lmin = (m_lbuf2[2] + m_lbuf2[3] + m_lbuf2[4]) / 3;
    m_lmax = (m_lbuf2[nbSymbols-3] + m_lbuf2[nbSymbols-4] + m_lbuf2[nbSymbols-5]) / 3;

    m_max = ((m_max) + m_lmax) / 2;
    m_min = ((m_min) + m_lmin) / 2;
    // recalibrate center/umid/lmid
    m_center = ((m_max) + (m_min)) / 2;
    m_umid = (((m_max) - m_center) * 5 / 8) + m_center;
    m_lmid = (((m_min) - m_center) * 5 / 8) + m_center;
}

void DSDSymbol::setFSK(unsigned int nbSymbols, bool inverted)
{
	if (nbSymbols == 2) { // binary FSK a.k.a. 2FSK
		m_nbFSKSymbols = 2;
	} else if (nbSymbols == 4) { // 4-ary FSK a.k.a. 4FSK
		m_nbFSKSymbols = 4;
	} else { // others are not supported => default to binary FSK
		m_nbFSKSymbols = 2;
	}

	m_invertedFSK = inverted;
}

int DSDSymbol::get_dibit_and_analog_signal(int* out_analog_signal)
{
    int symbol;
    int dibit;

    m_numflips = 0;
    symbol = m_symbol;

    if (out_analog_signal != 0) {
        *out_analog_signal = symbol;
    }

    use_symbol(symbol);

    dibit = digitize(symbol);

    return dibit;
}

void DSDSymbol::use_symbol(int symbol)
{
    if (m_dsdDecoder->m_state.dibit_buf_p > m_dsdDecoder->m_state.dibit_buf + 900000)
    {
        m_dsdDecoder->m_state.dibit_buf_p = m_dsdDecoder->m_state.dibit_buf + 200;
    }
}

int DSDSymbol::digitize(int symbol)
{
    // determine dibit state

	if (m_nbFSKSymbols == 2)
	{
        if (symbol > m_center)
        {
            *m_dsdDecoder->m_state.dibit_buf_p = 1; // store non-inverted values in dibit_buf
            m_dsdDecoder->m_state.dibit_buf_p++;
            return (m_invertedFSK ? 1 : 0);
        }
        else
        {
            *m_dsdDecoder->m_state.dibit_buf_p = 3; // store non-inverted values in dibit_buf
            m_dsdDecoder->m_state.dibit_buf_p++;
            return (m_invertedFSK ? 0 : 1);
        }
	}
	else if (m_nbFSKSymbols == 4)
	{
		int dibit;

        if (symbol > m_center)
        {
            if (symbol > m_umid)
            {
                dibit = m_invertedFSK ? 3 : 1; // -3 / +3
                *m_dsdDecoder->m_state.dibit_buf_p = 1; // store non-inverted values in dibit_buf
            }
            else
            {
                dibit = m_invertedFSK ? 2 : 0; // -1 / +1
                *m_dsdDecoder->m_state.dibit_buf_p = 0; // store non-inverted values in dibit_buf
            }
        }
        else
        {
            if (symbol < m_lmid)
            {
                dibit = m_invertedFSK ? 1 : 3;  // +3 / -3
                *m_dsdDecoder->m_state.dibit_buf_p = 3; // store non-inverted values in dibit_buf
            }
            else
            {
                dibit = m_invertedFSK ? 0 : 2;  // +1 / -1
                *m_dsdDecoder->m_state.dibit_buf_p = 2; // store non-inverted values in dibit_buf
            }
        }

        m_dsdDecoder->m_state.dibit_buf_p++;
        return dibit;
	}
	else // invalid
	{
        *m_dsdDecoder->m_state.dibit_buf_p = 0;
        m_dsdDecoder->m_state.dibit_buf_p++;
		return 0;
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

    sprintf(modulation, "GFSK");

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
                    if ((j == ((m_min) + 32768) / 1024)
                            || (j == ((m_max) + 32768) / 1024))
                    {
                        m_dsdDecoder->getLogger().log("#");
                    }
                    else if ((j == ((m_lmid) + 32768) / 1024)
                            || (j == ((m_umid) + 32768) / 1024))
                    {
                        m_dsdDecoder->getLogger().log("^");
                    }
                    else if (j == (m_center + 32768) / 1024)
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

int DSDSymbol::comp(const void *a, const void *b)
{
    if (*((const int *) a) == *((const int *) b))
        return 0;
    else if (*((const int *) a) < *((const int *) b))
        return -1;
    else
        return 1;
}


} // namespace DSDcc
