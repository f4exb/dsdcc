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
#include "dstar.h"
#include "dsd_decoder.h"
#include "descramble.h"

namespace DSDcc
{

const int DSDDstar::dW[72] = {

        // 0-11
        0, 0,
        3, 2,
        1, 1,
        0, 0,
        1, 1,
        0, 0,

        // 12-23
        3, 2,
        1, 1,
        3, 2,
        1, 1,
        0, 0,
        3, 2,

        // 24-35
        0, 0,
        3, 2,
        1, 1,
        0, 0,
        1, 1,
        0, 0,

        // 36-47
        3, 2,
        1, 1,
        3, 2,
        1, 1,
        0, 0,
        3, 2,

        // 48-59
        0, 0,
        3, 2,
        1, 1,
        0, 0,
        1, 1,
        0, 0,

        // 60-71
        3, 2,
        1, 1,
        3, 3,
        2, 1,
        0, 0,
        3, 3,
};

const int DSDDstar::dX[72] = {

        // 0-11
        10, 22,
        11, 9,
        10, 22,
        11, 23,
        8, 20,
        9, 21,

        // 12-23
        10, 8,
        9, 21,
        8, 6,
        7, 19,
        8, 20,
        9, 7,

        // 24-35
        6, 18,
        7, 5,
        6, 18,
        7, 19,
        4, 16,
        5, 17,

        // 36-47
        6, 4,
        5, 17,
        4, 2,
        3, 15,
        4, 16,
        5, 3,

        // 48-59
        2, 14,
        3, 1,
        2, 14,
        3, 15,
        0, 12,
        1, 13,

        // 60-71
        2, 0,
        1, 13,
        0, 12,
        10, 11,
        0, 12,
        1, 13,
};

DSDDstar::DSDDstar(DSDDecoder *dsdDecoder) :
        m_dsdDecoder(dsdDecoder)
{
}

DSDDstar::~DSDDstar()
{
}

void DSDDstar::init()
{
    fprintf(stderr, "DSDDstar::init: symbol %d (%d)\n", m_dsdDecoder->m_state.symbolcnt, m_dsdDecoder->m_dsdSymbol.getSymbol());

    if ((m_dsdDecoder->m_opts.errorbars == 1) && ((m_dsdDecoder->m_state.synctype == 6) || (m_dsdDecoder->m_state.synctype == 7)))
    {
        m_dsdDecoder->getLogger().log( "e:"); // print this only for voice/data frames
    }

    if (m_dsdDecoder->m_state.synctype == 18) {
        framecount = 0;
        m_dsdDecoder->m_state.synctype = 6;
    } else if (m_dsdDecoder->m_state.synctype == 19) {
        framecount = 0;
        m_dsdDecoder->m_state.synctype = 7;
    } else {
        framecount = 1; //just saw a sync frame; there should be 20 not 21 till the next
    }

    sync_missed = 0;
    bitbuffer = 0;
    m_symbolIndex = 0;
    m_symbolIndexHD = 0;
}

void DSDDstar::initVoiceFrame()
{
    //fprintf(stderr, "DSDDstar::initVoiceFrame\n");
    memset(ambe_fr, 0, 96);
    // voice frame
    w = dW;
    x = dX;
}
void DSDDstar::initDataFrame()
{
}

void DSDDstar::process()
{
    if (m_symbolIndex < 72) // voice frame
    {
        if (m_symbolIndex == 0) {
            initVoiceFrame();
        }

        processVoice();
    }
    else if (m_symbolIndex < 97) // data frame
    {
        if (m_symbolIndex == 72) {
            initDataFrame();
        }

        processData();
    }
    else
    {
        m_dsdDecoder->resetFrameSync(); // end
    }

    m_symbolIndex++;
}

void DSDDstar::processHD()
{
    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol and store it in HD cache
    radioheaderbuffer[m_symbolIndexHD] = dibit;

    if (m_symbolIndexHD == 660-1)
    {
        dstar_header_decode();
        m_symbolIndexHD = 0;
        m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDSTAR; // go to DSTAR
    }

    m_symbolIndexHD++;
}

void DSDDstar::processVoice()
{
    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol and store it in cache
    m_dibitCache[m_symbolIndex] = dibit;

    if (m_symbolIndex == 72-1) // last dibit in voice frame
    {
        int i;

        for (i = 0; i < 72; i++)
        {
            int dibit = m_dibitCache[i];

            bitbuffer <<= 1;

            if (dibit == 1) {
                bitbuffer |= 0x01;
            }

            if ((bitbuffer & 0x00FFFFFF) == 0x00AAB468)
            {
                // we're slipping bits
                m_dsdDecoder->getLogger().log( "sync in voice after i=%d, restarting\n", i);
                //ugh just start over
                m_symbolIndex = -1; // restart a frame sequence - incremented at the end of process() hence -1
                framecount = 1;
                break;
//                i = 0;
//                w = dW;
//                x = dX;
//                framecount = 1;
//                continue;
            }

            //fprintf(stderr, "DSDDstar::processVoice: i: %d w: %d x: %d\n", i, *w, *x);

            ambe_fr[*w][*x] = (1 & dibit);
            w++;
            x++;
        } // for

        if (i == 72) { // frame processed successfully
            m_dsdDecoder->m_mbeDecoder.processFrame(0, ambe_fr, 0);
        }
    }
}

void DSDDstar::processData()
{
    bool terminate = false;
    int dibit = m_dsdDecoder->m_dsdSymbol.getDibit(); // get dibit from symbol and store it in cache
    m_dibitCache[m_symbolIndex] = dibit;

    if (m_symbolIndex == 97-1) // last dibit in data frame
    {
        for (int i = 73; i < 97; i++)
        {
            int dibit = m_dibitCache[i];

            bitbuffer <<= 1;

            if (dibit == 1) {
                bitbuffer |= 0x01;
            }

            if ((bitbuffer & 0x00FFFFFF) == 0x00AAB468)
            {
                // looking if we're slipping bits
                if (i != 96)
                {
                    m_dsdDecoder->getLogger().log( "sync after i=%d\n", i);
                    break;
                }
            }
        }

        slowdata[0] = (bitbuffer >> 16) & 0x000000FF;
        slowdata[1] = (bitbuffer >> 8) & 0x000000FF;
        slowdata[2] = (bitbuffer) & 0x000000FF;
        slowdata[3] = 0;

        if ((bitbuffer & 0x00FFFFFF) == 0x00AAB468)
        {
            //We got sync!
            fprintf(stderr, "Sync on framecount = %d\n", framecount);
            sync_missed = 0;
        }
        else if ((bitbuffer & 0x00FFFFFF) == 0xAAAAAA)
        {
            //End of transmission
            m_dsdDecoder->getLogger().log("End of transmission\n");
            terminate = true;
        }
        else if (framecount % 21 == 0)
        {
            m_dsdDecoder->getLogger().log("Missed sync on framecount = %d, value = %x/%x/%x\n",
                    framecount, slowdata[0], slowdata[1], slowdata[2]);
            sync_missed++;
        }
        else if (framecount != 0 && (bitbuffer & 0x00FFFFFF) != 0x000000)
        {
            slowdata[0] ^= 0x70;
            slowdata[1] ^= 0x4f;
            slowdata[2] ^= 0x93;
            //printf("unscrambled- %s",slowdata);
        }
        else if (framecount == 0)
        {
            //printf("never scrambled-%s\n",slowdata);
        }

        //fprintf(stderr, "DSDDstar::processData: sync_missed: %d terminate: %d\n", sync_missed, (terminate ? 1 : 0));

        if (terminate)
        {
            m_dsdDecoder->resetFrameSync(); // end
        }
        else if (sync_missed < 3)
        {
            m_symbolIndex = -1; // restart a frame sequence - incremented at the end of process() hence -1
            framecount++;
            //fprintf(stderr, "DSDDstar::processData: restart frame: framecount: %d\n", framecount);
        }
        else
        {
            m_dsdDecoder->resetFrameSync(); // end
        }
    }
}

void DSDDstar::dstar_header_decode()
{
    int radioheaderbuffer2[660];
    unsigned char radioheader[41];
    int octetcount, bitcount, loop;
    unsigned char bit2octet[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    unsigned int FCSinheader;
    unsigned int FCScalculated;
    int len;

    Descramble::scramble(radioheaderbuffer, radioheaderbuffer2);
    Descramble::deinterleave(radioheaderbuffer2, radioheaderbuffer);
    len = Descramble::FECdecoder(radioheaderbuffer, radioheaderbuffer2);
    memset(radioheader, 0, 41);

    // note we receive 330 bits, but we only use 328 of them (41 octets)
    // bits 329 and 330 are unused
    octetcount = 0;
    bitcount = 0;

    for (loop = 0; loop < 328; loop++)
    {
        if (radioheaderbuffer2[loop])
        {
            radioheader[octetcount] |= bit2octet[bitcount];
        };

        bitcount++;

        // increase octetcounter and reset bitcounter every 8 bits
        if (bitcount >= 8)
        {
            octetcount++;
            bitcount = 0;
        }
    }

    // print header - TODO: store it in the state object for access from caller
    m_dsdDecoder->getLogger().log("\nDSTAR HEADER: ");
    //printf("FLAG1: %02X - FLAG2: %02X - FLAG3: %02X\n", radioheader[0],
    //      radioheader[1], radioheader[2]);
    m_dsdDecoder->getLogger().log("RPT 2: %c%c%c%c%c%c%c%c ", radioheader[3], radioheader[4],
            radioheader[5], radioheader[6], radioheader[7], radioheader[8],
            radioheader[9], radioheader[10]);
    m_dsdDecoder->getLogger().log("RPT 1: %c%c%c%c%c%c%c%c ", radioheader[11], radioheader[12],
            radioheader[13], radioheader[14], radioheader[15], radioheader[16],
            radioheader[17], radioheader[18]);
    m_dsdDecoder->getLogger().log("YOUR: %c%c%c%c%c%c%c%c ", radioheader[19], radioheader[20],
            radioheader[21], radioheader[22], radioheader[23], radioheader[24],
            radioheader[25], radioheader[26]);
    m_dsdDecoder->getLogger().log("MY: %c%c%c%c%c%c%c%c/%c%c%c%c\n", radioheader[27], radioheader[28],
            radioheader[29], radioheader[30], radioheader[31], radioheader[32],
            radioheader[33], radioheader[34], radioheader[35], radioheader[36],
            radioheader[37], radioheader[38]);

    // move to D-Star voice/data process on next sample
    init();
    m_dsdDecoder->m_fsmState = DSDDecoder::DSDprocessDSTAR;

    //FCSinheader = ((radioheader[39] << 8) | radioheader[40]) & 0xFFFF;
    //FCScalculated = calc_fcs((unsigned char*) radioheader, 39);
    //printf("Check sum = %04X ", FCSinheader);
    //if (FCSinheader == FCScalculated) {
    //  printf("(OK)\n");
    //} else {
    //  printf("(NOT OK- Calculated FCS = %04X)\n", FCScalculated);
    //}; // end else - if
}

} // namespace DSDcc
