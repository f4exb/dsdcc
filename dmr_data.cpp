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

#include "dmr_data.h"
#include "dsd_decoder.h"

namespace DSDcc
{

DSDDMRData::DSDDMRData(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder),
        m_symbolIndex(0)
{
}

DSDDMRData::~DSDDMRData()
{
}

void DSDDMRData::init()
{
    m_dsdDecoder->m_state.ccnum = 0;
    cc[4] = 0;
    bursttype[4] = 0;
    dibit_p = m_dsdDecoder->m_state.dibit_buf_p - 90;

    preProcess();

    m_symbolIndex = 0; // reset
}

void DSDDMRData::preProcess()
{
    int dibit;

    // CACH
    for (int i = 0; i < 12; i++)
    {
        dibit = *dibit_p;
        dibit_p++;

        if (m_dsdDecoder->m_opts.inverted_dmr == 1)
        {
            dibit = (dibit ^ 2);
        }

        cachdata[i] = dibit;

        if (i == 2)
        {
            m_dsdDecoder->m_state.currentslot = (1 & (dibit >> 1));      // bit 1
            if (m_dsdDecoder->m_state.currentslot == 0)
            {
                m_dsdDecoder->m_state.slot0light[0] = '[';
                m_dsdDecoder->m_state.slot0light[6] = ']';
                m_dsdDecoder->m_state.slot1light[0] = ' ';
                m_dsdDecoder->m_state.slot1light[6] = ' ';
            }
            else
            {
                m_dsdDecoder->m_state.slot1light[0] = '[';
                m_dsdDecoder->m_state.slot1light[6] = ']';
                m_dsdDecoder->m_state.slot0light[0] = ' ';
                m_dsdDecoder->m_state.slot0light[6] = ' ';
            }
        }
    }

    cachdata[12] = 0;

    // current slot
    dibit_p += 49;

    // slot type
    dibit = *dibit_p;
    dibit_p++;

    if (m_dsdDecoder->m_opts.inverted_dmr == 1)
    {
        dibit = (dibit ^ 2);
    }

    m_dsdDecoder->m_state.ccnum = dibit << 2;
    cc[0] = (1 & (dibit >> 1)) + 48;      // bit 1
    cc[1] = (1 & dibit) + 48;     // bit 0

    dibit = *dibit_p;
    dibit_p++;

    if (m_dsdDecoder->m_opts.inverted_dmr == 1)
    {
        dibit = (dibit ^ 2);
    }

    m_dsdDecoder->m_state.ccnum += dibit;
    cc[2] = (1 & (dibit >> 1)) + 48;      // bit 1
    cc[3] = (1 & dibit) + 48;     // bit 0

    dibit = *dibit_p;
    dibit_p++;

    if (m_dsdDecoder->m_opts.inverted_dmr == 1)
    {
        dibit = (dibit ^ 2);
    }

    bursttype[0] = (1 & (dibit >> 1)) + 48;       // bit 1
    bursttype[1] = (1 & dibit) + 48;      // bit 0

    dibit = *dibit_p;
    dibit_p++;

    if (m_dsdDecoder->m_opts.inverted_dmr == 1)
    {
        dibit = (dibit ^ 2);
    }

    bursttype[2] = (1 & (dibit >> 1)) + 48;       // bit 1
    bursttype[3] = (1 & dibit) + 48;      // bit 0

    // parity bit
    dibit_p++;

    if (strcmp(bursttype, "0000") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " PI Header    ");
    }
    else if (strcmp(bursttype, "0001") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " VOICE Header ");
    }
    else if (strcmp(bursttype, "0010") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " TLC          ");
    }
    else if (strcmp(bursttype, "0011") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " CSBK         ");
    }
    else if (strcmp(bursttype, "0100") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " MBC Header   ");
    }
    else if (strcmp(bursttype, "0101") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " MBC          ");
    }
    else if (strcmp(bursttype, "0110") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " DATA Header  ");
    }
    else if (strcmp(bursttype, "0111") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " RATE 1/2 DATA");
    }
    else if (strcmp(bursttype, "1000") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " RATE 3/4 DATA");
    }
    else if (strcmp(bursttype, "1001") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " Slot idle    ");
    }
    else if (strcmp(bursttype, "1010") == 0)
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, " Rate 1 DATA  ");
    }
    else
    {
        sprintf(m_dsdDecoder->m_state.fsubtype, "              ");
    }

    // signaling data or sync
    for (int i = 0; i < 24; i++)
    {
        dibit = *dibit_p;
        dibit_p++;

        if (m_dsdDecoder->m_opts.inverted_dmr == 1)
        {
            dibit = (dibit ^ 2);
        }

        syncdata[i] = dibit;
        sync[i] = (dibit | 1) + 48;
    }

    sync[24] = 0;
    syncdata[24] = 0;

    if ((strcmp(sync, DMR_BS_DATA_SYNC) == 0)
     || (strcmp(sync, DMR_MS_DATA_SYNC) == 0))
    {
        if (m_dsdDecoder->m_state.currentslot == 0)
        {
            sprintf(m_dsdDecoder->m_state.slot0light, "[slot0]");
        }
        else
        {
            sprintf(m_dsdDecoder->m_state.slot1light, "[slot1]");
        }
    }

    if (m_dsdDecoder->m_opts.errorbars == 1)
    {
        m_dsdDecoder->getLogger().log("%s %s\n", m_dsdDecoder->m_state.slot0light, m_dsdDecoder->m_state.slot1light);
    }
}

void DSDDMRData::process()
{
    m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol but do nothing with it

    if (m_symbolIndex == 120-1) // last dibit to skip
    {
        if (m_dsdDecoder->m_opts.errorbars == 1)
        {
            if (strcmp(m_dsdDecoder->m_state.fsubtype, "              ") == 0)
            {
                m_dsdDecoder->getLogger().log(" Unknown burst type: %s\n", bursttype);
            }
            else
            {
                m_dsdDecoder->getLogger().log("%s\n", m_dsdDecoder->m_state.fsubtype);
            }
        }

        m_dsdDecoder->resetFrameSync(); // get ready for next sync
    }

    m_symbolIndex++;
}

} // namespace DSDcc
