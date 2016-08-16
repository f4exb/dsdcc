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
#include "dsd_decoder.h"

namespace DSDcc
{

DSDDecoder::DSDDecoder() :
        m_fsmState(DSDLookForSync),
        m_hasSync(false),
        m_mbeDVReady(false),
        m_mbelibEnable(true),
        m_mbeRate(DSDMBERateNone),
        m_dsdSymbol(this),
        m_mbeDecoder(this),
        m_dsdDMRVoice(this),
        m_dsdDMRData(this),
        m_dsdDstar(this),
        m_dsdYSF(this),
        m_dsdDPMR(this),
		m_dsdNXDN(this),
        m_dataRate(DSDRate4800)
{
    resetFrameSync();
    noCarrier();
    m_squelchTimeoutCount = 0;
}

DSDDecoder::~DSDDecoder()
{
}

void DSDDecoder::setQuiet()
{
    m_opts.errorbars = 0;
    m_opts.verbose = 0;
    m_dsdLogger.setVerbosity(0);
}

void DSDDecoder::setVerbosity(int verbosity)
{
    m_opts.verbose = verbosity;
    m_dsdLogger.setVerbosity(verbosity);
}

void DSDDecoder::showDatascope()
{
    m_opts.errorbars = 0;
    m_opts.p25enc = 0;
    m_opts.p25lc = 0;
    m_opts.p25status = 0;
    m_opts.p25tg = 0;
    m_opts.datascope = 1;
    m_opts.symboltiming = 0;
}

void DSDDecoder::setDatascopeFrameRate(int frameRate)
{
    m_opts.errorbars = 0;
    m_opts.p25enc = 0;
    m_opts.p25lc = 0;
    m_opts.p25status = 0;
    m_opts.p25tg = 0;
    m_opts.datascope = 1;
    m_opts.symboltiming = 0;
    m_opts.scoperate = frameRate;
    m_dsdLogger.log("Setting datascope frame rate to %i frame per second.\n", frameRate);
}

void DSDDecoder::showErrorBars()
{
    m_opts.errorbars = 1;
    m_opts.datascope = 0;
}

void DSDDecoder::showSymbolTiming()
{
    m_opts.symboltiming = 1;
    m_opts.errorbars = 1;
    m_opts.datascope = 0;
}

void DSDDecoder::setP25DisplayOptions(DSDShowP25 mode, bool on)
{
    switch(mode)
    {
    case DSDShowP25EncryptionSyncBits:
        m_opts.p25enc = (on ? 1 : 0);
        break;
    case DSDShowP25LinkControlBits:
        m_opts.p25lc = (on ? 1 : 0);
        break;
    case DSDShowP25StatusBitsAndLowSpeedData:
        m_opts.p25status = (on ? 1 : 0);
        break;
    case DSDShowP25TalkGroupInfo:
        m_opts.p25tg = (on ? 1 : 0);
        break;
    default:
        break;
    }
}

void DSDDecoder::muteEncryptedP25(bool on)
{
    m_opts.unmute_encrypted_p25 = (on ? 0 : 1);
}

void DSDDecoder::setDecodeMode(DSDDecodeMode mode, bool on)
{
    switch(mode)
    {
    case DSDDecodeNone:
        if (on)
        {
            m_opts.frame_dmr = 0;
            m_opts.frame_dstar = 0;
            m_opts.frame_p25p1 = 0;
            m_opts.frame_nxdn48 = 0;
            m_opts.frame_nxdn96 = 0;
            m_opts.frame_provoice = 0;
            m_opts.frame_x2tdma = 0;
            m_opts.frame_dpmr = 0;
            m_opts.frame_ysf = 0;
        }
        break;
    case DSDDecodeDMR:
        m_opts.frame_dmr = (on ? 1 : 0);
        if (on) setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of DMR/MOTOTRBO frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeDStar:
        m_opts.frame_dstar = (on ? 1 : 0);
        if (on) setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of D-Star frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeP25P1:
        m_opts.frame_p25p1 = (on ? 1 : 0);
        if (on) setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of P25p1 frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeDPMR:
        m_opts.frame_dpmr = (on ? 1 : 0);
        if (on) setDataRate(DSDRate2400); else setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of DPMR Tier 1 or 2 frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeNXDN48:
        m_opts.frame_nxdn48 = (on ? 1 : 0);
        if (on) setDataRate(DSDRate2400); else setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of NXDN48 frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeNXDN96:
        m_opts.frame_nxdn96 = (on ? 1 : 0);
        if (on) setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of NXDN96 frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeProVoice:
        m_opts.frame_provoice = (on ? 1 : 0);
        if (on) setDataRate(DSDRate9600); else setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of Pro Voice frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeX2TDMA:
        m_opts.frame_x2tdma = (on ? 1 : 0);
        if (on) setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of X2 TDMA frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeYSF:
        m_opts.frame_ysf = (on ? 1 : 0);
        if (on) setDataRate(DSDRate4800);
        m_dsdLogger.log("%s the decoding of YSF frames.\n", (on ? "Enabling" : "Disabling"));
        break;
    case DSDDecodeAuto:
        m_opts.frame_dmr = 0;
        m_opts.frame_dstar = 0;
        m_opts.frame_p25p1 = 0;
        m_opts.frame_nxdn48 = 0;
        m_opts.frame_nxdn96 = 0;
        m_opts.frame_provoice = 0;
        m_opts.frame_x2tdma = 0;
        m_opts.frame_dpmr = 0;
        m_opts.frame_ysf = 0;
        switch (m_dataRate)
        {
        case DSDRate2400:
            m_opts.frame_nxdn48 = (on ? 1 : 0);
            m_opts.frame_dpmr = (on ? 1 : 0);
            break;
        case DSDRate4800:
            m_opts.frame_dmr = (on ? 1 : 0);
            m_opts.frame_dstar = (on ? 1 : 0);
            m_opts.frame_x2tdma = (on ? 1 : 0);
            m_opts.frame_p25p1 = (on ? 1 : 0);
            m_opts.frame_nxdn96 = (on ? 1 : 0);
            m_opts.frame_ysf = (on ? 1 : 0);
            break;
        case DSDRate9600:
            m_opts.frame_provoice = (on ? 1 : 0);
            break;
        default:
            m_opts.frame_dmr = (on ? 1 : 0);
            m_opts.frame_dstar = (on ? 1 : 0);
            m_opts.frame_x2tdma = (on ? 1 : 0);
            m_opts.frame_p25p1 = (on ? 1 : 0);
            m_opts.frame_nxdn96 = (on ? 1 : 0);
            m_opts.frame_ysf = (on ? 1 : 0);
            break;
        }
        m_dsdLogger.log("%s auto frame decoding.\n", (on ? "Enabling" : "Disabling"));
        break;
    default:
        break;
    }
}

void DSDDecoder::setModulationOptimizations(DSDModulationOptim mode)
{
    m_opts.mod_c4fm = 0;
    m_opts.mod_qpsk = 0;
    m_opts.mod_gfsk = 1;
    m_state.rf_mod = 2;
    m_dsdLogger.log("Enabling only GFSK modulation optimizations.\n");
//
//
//    switch (mode)
//    {
//    case DSDModulationOptimAuto:
//        m_opts.mod_c4fm = 1;
//        m_opts.mod_qpsk = 1;
//        m_opts.mod_gfsk = 1;
//        m_state.rf_mod = 0;
//        m_dsdLogger.log("Enabling Auto modulation optimizations.\n");
//        break;
//    case DSDModulationOptimGFSK:
//        m_opts.mod_c4fm = 0;
//        m_opts.mod_qpsk = 0;
//        m_opts.mod_gfsk = 1;
//        m_state.rf_mod = 2;
//        m_dsdLogger.log("Enabling only GFSK modulation optimizations.\n");
//        break;
//    case DSDModulationOptimC4FM:
//        m_opts.mod_c4fm = 1;
//        m_opts.mod_qpsk = 0;
//        m_opts.mod_gfsk = 0;
//        m_state.rf_mod = 0;
//        m_dsdLogger.log("Enabling only C4FM modulation optimizations.\n");
//        break;
//    default:
//        break;
//    }
}

void DSDDecoder::setAudioGain(float gain)
{
    m_opts.audio_gain = gain;

    if (m_opts.audio_gain < 0.0f)
    {
        m_dsdLogger.log("Disabling audio out gain setting\n");
    }
    else if (m_opts.audio_gain == 0.0f)
    {
        m_opts.audio_gain = 0.0f;
        m_dsdLogger.log("Enabling audio out auto-gain\n");
    }
    else
    {
        m_dsdLogger.log("Setting audio out gain to %f\n", m_opts.audio_gain);
        m_state.aout_gain = m_opts.audio_gain;
    }
}

void DSDDecoder::setUvQuality(int uvquality)
{
    m_opts.uvquality = uvquality;

    if (m_opts.uvquality < 1) {
        m_opts.uvquality = 1;
    } else if (m_opts.uvquality > 64) {
        m_opts.uvquality = 64;
    }

    m_dsdLogger.log("Setting unvoice speech quality to %i waves per band.\n", m_opts.uvquality);
}

void DSDDecoder::setUpsampling(int upsampling)
{
    m_opts.upsample = upsampling;

    if ((m_opts.upsample != 6) && (m_opts.upsample != 7)) {
        m_opts.upsample = 0;
    }

    m_dsdLogger.log("Setting upsampling to x%d\n", (m_opts.upsample == 0 ? 1 : m_opts.upsample));
}

void DSDDecoder::setStereo(bool on)
{
    m_opts.stereo = (on ? 1 : 0);
}

void DSDDecoder::setInvertedXTDMA(bool on)
{
    m_opts.inverted_x2tdma = (on ? 1 : 0);
    m_dsdLogger.log("Expecting %sinverted X2-TDMA signals.\n", (m_opts.inverted_x2tdma == 0 ? "non-" : ""));
}

void DSDDecoder::setInvertedDMR(bool on)
{
    m_opts.inverted_dmr = (on ? 1 : 0);
    m_dsdLogger.log("Expecting %sinverted DMR/MOTOTRBO signals.\n", (m_opts.inverted_x2tdma == 0 ? "non-" : ""));
}

void DSDDecoder::setAutoDetectionThreshold(int threshold)
{
    m_opts.mod_threshold = threshold;
    m_dsdLogger.log("Setting C4FM/QPSK auto detection threshold to %i\n", m_opts.mod_threshold);
}

void DSDDecoder::setQPSKSymbolBufferSize(int size)
{
    m_opts.ssize = size;

    if (m_opts.ssize > 128) {
        m_opts.ssize = 128;
    } else if (m_opts.ssize < 1) {
        m_opts.ssize = 1;
    }

    m_dsdLogger.log("Setting QPSK symbol buffer to %i\n", m_opts.ssize);
}

void DSDDecoder::setQPSKMinMaxBufferSize(int size)
{
    m_opts.msize = size;

    if (m_opts.msize > 1024) {
        m_opts.msize = 1024;
    } else if (m_opts.msize < 1) {
        m_opts.msize = 1;
    }

    m_dsdLogger.log("Setting QPSK Min/Max buffer to %i\n", m_opts.msize);
}

void DSDDecoder::enableCosineFiltering(bool on)
{
    m_opts.use_cosine_filter = (on ? 1 : 0);
    m_dsdLogger.log("%s cosine filter.\n", (on ? "Enabling" : "Disabling"));
}

void DSDDecoder::enableAudioOut(bool on)
{
    m_opts.audio_out = (on ? 1 : 0);
    m_dsdLogger.log("%s audio output to soundcard.\n", (on ? "Enabling" : "Disabling"));
}

void DSDDecoder::enableScanResumeAfterTDULCFrames(int nbFrames)
{
    m_opts.resume = nbFrames;
    m_dsdLogger.log("Enabling scan resume after %i TDULC frames\n", m_opts.resume);
}

void DSDDecoder::setDataRate(DSDRate dataRate)
{
    m_dataRate = dataRate;

    switch(dataRate)
    {
    case DSDRate2400:
        m_dsdLogger.log("Set data rate to 2400 bauds. 20 samples per symbol\n");
        m_state.samplesPerSymbol = 20;
        m_state.symbolCenter = 10;
        break;
    case DSDRate4800:
        m_dsdLogger.log("Set data rate to 4800 bauds. 10 samples per symbol\n");
        m_state.samplesPerSymbol = 10;
        m_state.symbolCenter = 4;
        break;
    case DSDRate9600:
        m_dsdLogger.log("Set data rate to 9600 bauds. 5 samples per symbol\n");
        m_state.samplesPerSymbol = 5;
        m_state.symbolCenter = 2;
        break;
    default:
        m_dsdLogger.log("Set default data rate to 4800 bauds. 10 samples per symbol\n");
        m_state.samplesPerSymbol = 10;
        m_state.symbolCenter = 4;
        break;
    }
}

void DSDDecoder::run(short sample)
{
    // mode time out if squelch has been closed for a number of samples
    if (m_fsmState != DSDLookForSync)
    {
        if (sample == 0)
        {
            if (m_squelchTimeoutCount < DSD_SQUELCH_TIMEOUT_SAMPLES)
            {
                m_squelchTimeoutCount++;
            }
            else
            {
                m_dsdLogger.log("DSDDecoder::run: squelch time out go back to sync search\n");
                resetFrameSync();
                m_squelchTimeoutCount = 0;
            }
        }
    }

    if (m_dsdSymbol.pushSample(sample, m_hasSync)) // a symbol is retrieved
    {
        switch (m_fsmState)
        {
        case DSDLookForSync:
            m_sync = getFrameSync(); // -> -2: still looking, -1 not found, 0 and above: sync found

            if (m_sync > -2) // -1 and above means syncing has been processed (sync found or not but not searching)
            {
                m_dsdLogger.log("DSDDecoder::run: sync found: %d symbol %d (%d)\n", m_sync, m_state.symbolcnt, m_dsdSymbol.getSymbol());
                // recalibrate center/umid/lmid
                m_state.center = ((m_state.max) + (m_state.min)) / 2;
                m_state.umid = (((m_state.max) - m_state.center) * 5 / 8) + m_state.center;
                m_state.lmid = (((m_state.min) - m_state.center) * 5 / 8) + m_state.center;

                if (m_sync > -1) // good sync found
                {
                    m_hasSync = true;
                    m_fsmState = DSDSyncFound; // go to processing state next time
                }
                else // no sync found
                {
                    resetFrameSync(); // go back searching
                }
            }
            // still searching -> no change in FSM state
            break;
        case DSDSyncFound:
            m_state.synctype  = m_sync;
            if (m_state.synctype > -1) // 0 and above means a sync has been found
            {
                m_dsdLogger.log("DSDDecoder::run: before processFrameInit: symbol %d (%d)\n", m_state.symbolcnt, m_dsdSymbol.getSymbol());
                processFrameInit();   // initiate the process of the frame which sync has been found. This will change FSM state
            }
            else // no sync has been found after searching -> call noCarrier() and go back searching
            {
                noCarrier();
                resetFrameSync();
                //m_fsmState = DSDLookForSync;
            }
            break;
        case DSDprocessDMRvoice:
            m_dsdDMRVoice.process();
            break;
        case DSDprocessDMRdata:
            m_dsdDMRData.process();
            break;
        case DSDprocessDSTAR:
            m_dsdDstar.process();
            break;
        case DSDprocessDSTAR_HD:
            m_dsdDstar.processHD();
            break;
        case DSDprocessYSF:
            m_dsdYSF.process();
            break;
        case DSDprocessDPMR:
            m_dsdDPMR.process();
            break;
        case DSDprocessNXDN:
            m_dsdNXDN.process();
            break;
        default:
            break;
        }
    }
}

void DSDDecoder::processFrameInit()
{
	m_state.maxref = m_state.max;
	m_state.minref = m_state.min;

    if ((m_state.synctype >= 10) && (m_state.synctype <= 13)) // DMR
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = (int) m_state.max / 164;
                m_dsdLogger.log("inlvl: %2i%% ", level);
            }
        }

        if ((m_state.synctype == 11) || (m_state.synctype == 12))
        {
            sprintf(m_state.fsubtype, " VOICE        ");
            m_dsdDMRVoice.init();    // initializations not consuming a live symbol
            m_dsdDMRVoice.process(); // process current symbol first
            m_fsmState = DSDprocessDMRvoice;
        }
        else
        {
            m_state.err_str[0] = 0;
            m_dsdDMRData.init();    // initializations not consuming a live symbol
            m_dsdDMRData.process(); // process current symbol first
            m_fsmState = DSDprocessDMRdata;
        }
    }
    else if ((m_state.synctype == 6) || (m_state.synctype == 7)) // D-Star voice
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = (int) m_state.max / 164;
                printf("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " VOICE        ");
        m_dsdDstar.init();
        m_dsdDstar.process(); // process current symbol first
        m_fsmState = DSDprocessDSTAR;
    }
    else if ((m_state.synctype == 8) || (m_state.synctype == 9)) // NXDN conventional
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = (int) m_state.max / 164;
                printf("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " RDCH         ");
        m_dsdNXDN.init();
        m_dsdNXDN.process(); // process current symbol first
        m_fsmState = DSDprocessNXDN;
    }
    else if ((m_state.synctype == 18) || (m_state.synctype == 19)) // D-Star header
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = (int) m_state.max / 164;
                m_dsdLogger.log("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " DATA         ");
        m_dsdDstar.init();
        m_dsdDstar.processHD(); // process current symbol first
        m_fsmState = DSDprocessDSTAR_HD;
    }
    else if (m_state.synctype == 20) // dPMR classic (not packet)
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = (int) m_state.max / 164;
                m_dsdLogger.log("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " ANY          ");
        m_dsdDPMR.init();
        m_dsdDPMR.process();
        m_fsmState = DSDprocessDPMR;
    }
    else if (m_state.synctype == 24) // YSF
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = (int) m_state.max / 164;
                m_dsdLogger.log("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " ANY          ");
        m_dsdYSF.init();
        m_dsdYSF.process();
        m_fsmState = DSDprocessYSF;
    }
    else
    {
        noCarrier();
        m_fsmState = DSDLookForSync;
    }
}

int DSDDecoder::getFrameSync()
{
    /* detects frame sync and returns frame type
     * -2 = in progress
     * -1 = no sync
     * 0  = +P25p1
     * 1  = -P25p1
     * 2  = +X2-TDMA (non inverted signal data frame)
     * 3  = +X2-TDMA (inverted signal voice frame)
     * 4  = -X2-TDMA (non inverted signal voice frame)
     * 5  = -X2-TDMA (inverted signal data frame)
     * 6  = +D-STAR
     * 7  = -D-STAR
     * 8  = +NXDN (non inverted RDCH first frame)
     * 9  = -NXDN (inverted RDCH first frame)
     * 10 = +DMR (non inverted singlan data frame)
     * 11 = -DMR (inverted signal voice frame)
     * 12 = +DMR (non inverted signal voice frame)
     * 13 = -DMR (inverted signal data frame)
     * 14 = +ProVoice
     * 15 = -ProVoice
     * 16 = +NXDN (non inverted data frame - not used)
     * 17 = -NXDN (inverted data frame - not used)
     * 18 = +D-STAR_HD
     * 19 = -D-STAR_HD
     * 20 = +dPMR Tier 1 or 2 FS1 (just sync detection - not implemented yet)
     * 21 = +dPMR Tier 1 or 2 FS4 (just sync detection - not implemented yet)
     * 22 = +dPMR Tier 1 or 2 FS2 (handled by specialized class - not used)
     * 23 = +dPMR Tier 1 or 2 FS3 (handled by specialized class - not used)
     * 24 = +YSF (just sync detection - not implemented yet)
     */

    // smelly while was starting here
    //symbol = getSymbol(opts, state, 0);
    m_t++;
    m_state.sbuf[m_state.sidx] = m_dsdSymbol.getSymbol();

    if (m_state.sidx == (m_opts.ssize - 1))
    {
        m_state.sidx = 0;
    }
    else
    {
        m_state.sidx++;
    }

    if (m_state.dibit_buf_p > m_state.dibit_buf + 900000)
    {
        m_state.dibit_buf_p = m_state.dibit_buf + 200;
    }

    //determine dibit state
    if (m_dsdSymbol.getSymbol() > 0)
    {
        *m_state.dibit_buf_p = 1;
        m_state.dibit_buf_p++;
        m_dibit = 49; // ASCII character '1'
    }
    else
    {
        *m_state.dibit_buf_p = 3;
        m_state.dibit_buf_p++;
        m_dibit = 51; // ASCII character '3'
    }

    *m_synctest_p = m_dibit;

    if (m_t >= 18)
    {
        // Just update for display

        if (m_state.rf_mod == 0)
        {
            sprintf(m_modulation, "C4FM");
        }
        else if (m_state.rf_mod == 2)
        {
            sprintf(m_modulation, "GFSK");
        }

        // Sync identification starts here

        strncpy(m_synctest, (m_synctest_p - 23), 24);
        m_stationType = DSDStationTypeNotApplicable;

        if (m_opts.frame_p25p1 == 1)
        {
            if (strcmp(m_synctest, P25P1_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "+P25 Phase 1 ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +P25p1    ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 0;
                m_mbeRate = DSDMBERate3600x2450;
                return(0);
            }
            if (strcmp(m_synctest, INV_P25P1_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "-P25 Phase 1 ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" -P25p1    ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 1;
                m_mbeRate = DSDMBERate3600x2450;
                return(1);
            }
        }
        if (m_opts.frame_x2tdma == 1)
        {
            if ((strcmp(m_synctest, X2TDMA_BS_DATA_SYNC) == 0)
             || (strcmp(m_synctest, X2TDMA_MS_DATA_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                if (strcmp(m_synctest, X2TDMA_BS_DATA_SYNC) == 0) {
                    m_stationType = DSDBaseStation;
                } else {
                    m_stationType = DSDMobileStation;
                }

                if (m_opts.inverted_x2tdma == 0)
                {
                    // data frame
                    sprintf(m_state.ftype, "+X2-TDMAd    ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" +X2-TDMA  ", m_synctest_pos + 1, m_modulation);
                    }

                    m_state.lastsynctype = 2;
                    m_mbeRate = DSDMBERate3600x2450;
                    return(2); // done
                }
                else
                {
                    // inverted voice frame
                    sprintf(m_state.ftype, "-X2-TDMAv    ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" -X2-TDMA  ", m_synctest_pos + 1, m_modulation);
                    }

                    if (m_state.lastsynctype != 3)
                    {
                        m_state.firstframe = 1;
                    }

                    m_state.lastsynctype = 3;
                    m_mbeRate = DSDMBERate3600x2450;
                    return(3); // done
                }
            }
            if ((strcmp(m_synctest, X2TDMA_BS_VOICE_SYNC) == 0)
             || (strcmp(m_synctest, X2TDMA_MS_VOICE_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                if (strcmp(m_synctest, X2TDMA_BS_VOICE_SYNC) == 0) {
                    m_stationType = DSDBaseStation;
                } else {
                    m_stationType = DSDMobileStation;
                }

                if (m_opts.inverted_x2tdma == 0)
                {
                    // voice frame
                    sprintf(m_state.ftype, "+X2-TDMAv    ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" +X2-TDMA  ", m_synctest_pos + 1, m_modulation);
                    }

                    if (m_state.lastsynctype != 4)
                    {
                        m_state.firstframe = 1;
                    }

                    m_state.lastsynctype = 4;
                    m_mbeRate = DSDMBERate3600x2450;
                    return(4); // done
                }
                else
                {
                    // inverted data frame
                    sprintf(m_state.ftype, "-X2-TDMAd    ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" -X2-TDMA  ", m_synctest_pos + 1, m_modulation);
                    }

                    m_state.lastsynctype = 5;
                    m_mbeRate = DSDMBERate3600x2450;
                    return(5); // done
                }
            }
        }
        if (m_opts.frame_ysf == 1)
        {
            strncpy(m_synctest20, (m_synctest_p - 19), 20);

            if (strcmp(m_synctest20, YSF_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(20);

                sprintf(m_state.ftype, "+YSF         ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("+YSF       ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 24;
                return(24);
            }
        }
        if (m_opts.frame_dmr == 1)
        {
            if ((strcmp(m_synctest, DMR_MS_DATA_SYNC) == 0)
             || (strcmp(m_synctest, DMR_BS_DATA_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                if (strcmp(m_synctest, DMR_BS_DATA_SYNC) == 0) {
                    m_stationType = DSDBaseStation;
                } else {
                    m_stationType = DSDMobileStation;
                }

                if (m_opts.inverted_dmr == 0)
                {
                    // data frame
                    sprintf(m_state.ftype, "+DMRd        ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" +DMRd     ",  m_synctest_pos + 1, m_modulation);
                    }

                    m_state.lastsynctype = 10;
                    m_mbeRate = DSDMBERate3600x2450;
                    return(10); // done
                }
                else
                {
                    // inverted voice frame
                    sprintf(m_state.ftype, "-DMRv        ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" -DMRv     ", m_synctest_pos + 1, m_modulation);
                    }

                    if (m_state.lastsynctype != 11)
                    {
                        m_state.firstframe = 1;
                    }

                    m_state.lastsynctype = 11;
                    m_mbeRate = DSDMBERate3600x2450;
                    return(11); // done
                }
            }
            if ((strcmp(m_synctest, DMR_MS_VOICE_SYNC) == 0)
             || (strcmp(m_synctest, DMR_BS_VOICE_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                if (strcmp(m_synctest, DMR_BS_VOICE_SYNC) == 0) {
                    m_stationType = DSDBaseStation;
                } else {
                    m_stationType = DSDMobileStation;
                }

                if (m_opts.inverted_dmr == 0)
                {
                    // voice frame
                    sprintf(m_state.ftype, "+DMRv        ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" +DMRv     ", m_synctest_pos + 1, m_modulation);
                    }

                    if (m_state.lastsynctype != 12)
                    {
                        m_state.firstframe = 1;
                    }

                    m_state.lastsynctype = 12;
                    m_mbeRate = DSDMBERate3600x2450;
                    return(12); // done
                }
                else
                {
                    // inverted data frame
                    sprintf(m_state.ftype, "-DMRd        ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" -DMRd     ", m_synctest_pos + 1, m_modulation);
                    }

                    m_state.lastsynctype = 13;
                    m_mbeRate = DSDMBERate3600x2450;
                    return(13); // done
                }
            }
        }
        if (m_opts.frame_provoice == 1)
        {
            strncpy(m_synctest32, (m_synctest_p - 31), 32);

            if ((strcmp(m_synctest32, PROVOICE_SYNC) == 0)
             || (strcmp(m_synctest32, PROVOICE_EA_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(32);

                sprintf(m_state.ftype, "+ProVoice    ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +ProVoice ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 14;
                m_mbeRate = DSDMBERate3600x2450;
                return(14); // done
            }
            else if ((strcmp(m_synctest32, INV_PROVOICE_SYNC) == 0)
                  || (strcmp(m_synctest32, INV_PROVOICE_EA_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(32);

                sprintf(m_state.ftype, "-ProVoice    ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" -ProVoice ",  m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 15;
                m_mbeRate = DSDMBERate3600x2450;
                return(15); // donesynctype
            }

        }
        if ((m_opts.frame_nxdn96 == 1) || (m_opts.frame_nxdn48 == 1))
        {
        	strncpy(m_synctest20, (m_synctest_p - 19), 20);

            if (strcmp(m_synctest20, NXDN_RDCH_FULL_SYNC) == 0)
            {
				m_state.carrier = 1;
				m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(20);

				if (m_state.samplesPerSymbol == 20)
				{
					sprintf(m_state.ftype, "+NXDN48      ");

					if (m_opts.errorbars == 1)
					{
						printFrameSync(" +NXDN48   ", m_synctest_pos + 1, m_modulation);
					}
				}
				else
				{
					sprintf(m_state.ftype, "+NXDN96      ");

					if (m_opts.errorbars == 1)
					{
						printFrameSync(" +NXDN96   ", m_synctest_pos + 1, m_modulation);
					}
				}

				m_state.lastsynctype = 8;
				m_mbeRate = DSDMBERate3600x2450;
				return(8); // done
            }
            else if (strcmp(m_synctest20, INV_NXDN_RDCH_FULL_SYNC) == 0)
            {
				m_state.carrier = 1;
				m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(20);

				if (m_state.samplesPerSymbol == 20)
				{
					sprintf(m_state.ftype, "-NXDN48      ");

					if (m_opts.errorbars == 1)
					{
						printFrameSync(" -NXDN48   ", m_synctest_pos + 1, m_modulation);
					}
				}
				else
				{
					sprintf(m_state.ftype, "-NXDN96      ");

					if (m_opts.errorbars == 1)
					{
						printFrameSync(" -NXDN96   ", m_synctest_pos + 1, m_modulation);
					}
				}

				m_state.lastsynctype = 9;
				m_mbeRate = DSDMBERate3600x2450;
				return(9); // done
            }
//            else if ((strcmp(m_synctest18, NXDN_BS_DATA_SYNC) == 0)
//                  || (strcmp(m_synctest18, NXDN_MS_DATA_SYNC) == 0))
//            {
//                if (strcmp(m_synctest, NXDN_BS_DATA_SYNC) == 0) {
//                    m_stationType = DSDBaseStation;
//                } else {
//                    m_stationType = DSDMobileStation;
//                }
//
////                if ((m_state.lastsynctype == 8)
////                        || (m_state.lastsynctype == 16))
//                {
//                    m_state.carrier = 1;
//                    m_state.offset = m_synctest_pos;
//                    m_state.max = ((m_state.max) + m_lmax) / 2;
//                    m_state.min = ((m_state.min) + m_lmin) / 2;
//
//                    if (m_state.samplesPerSymbol == 20)
//                    {
//                        sprintf(m_state.ftype, "+NXDN48d     ");
//
//                        if (m_opts.errorbars == 1)
//                        {
//                            printFrameSync(" +NXDN48   ", m_synctest_pos + 1, m_modulation);
//                        }
//                    }
//                    else
//                    {
//                        sprintf(m_state.ftype, "+NXDN96d     ");
//
//                        if (m_opts.errorbars == 1)
//                        {
//                            printFrameSync(" +NXDN96   ", m_synctest_pos + 1, m_modulation);
//                        }
//                    }
//
//                    m_state.lastsynctype = 16;
//                    m_mbeRate = DSDMBERate3600x2450;
//                    return(16); // done
//                }
////                else
////                {
////                    m_state.lastsynctype = 16;
////                }
//            }
//            else if ((strcmp(m_synctest18, INV_NXDN_BS_DATA_SYNC) == 0)
//                  || (strcmp(m_synctest18, INV_NXDN_MS_DATA_SYNC) == 0))
//            {
//                if (strcmp(m_synctest, INV_NXDN_BS_DATA_SYNC) == 0) {
//                    m_stationType = DSDBaseStation;
//                } else {
//                    m_stationType = DSDMobileStation;
//                }
//
////                if ((m_state.lastsynctype == 9)
////                        || (m_state.lastsynctype == 17))
//                {
//                    m_state.carrier = 1;
//                    m_state.offset = m_synctest_pos;
//                    m_state.max = ((m_state.max) + m_lmax) / 2;
//                    m_state.min = ((m_state.min) + m_lmin) / 2;
//
//                    sprintf(m_state.ftype, "-NXDN        ");
//
//                    if (m_state.samplesPerSymbol == 20)
//                    {
//                        sprintf(m_state.ftype, "-NXDN48d     ");
//
//                        if (m_opts.errorbars == 1)
//                        {
//                            printFrameSync(" -NXDN48   ", m_synctest_pos + 1, m_modulation);
//                        }
//                    }
//                    else
//                    {
//                        sprintf(m_state.ftype, "-NXDN96d     ");
//
//                        if (m_opts.errorbars == 1)
//                        {
//                            printFrameSync(" -NXDN96   ", m_synctest_pos + 1, m_modulation);
//                        }
//                    }
//
//                    m_state.lastsynctype = 17;
//                    m_mbeRate = DSDMBERate3600x2450;
//                    return(17); // done
//                }
////                else
////                {
////                    m_state.lastsynctype = 17;
////                }
//            }
//        }
        }
        if (m_opts.frame_dpmr == 1)
        {
            if (strcmp(m_synctest, DPMR_FS1_SYNC) == 0) // dPMR classic (not packet)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "+dPMR        ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("+dPMR      ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 20;
                m_mbeRate = DSDMBERate3600x2450;
                return(20);
            }
            else if (strcmp(m_synctest, DPMR_FS4_SYNC) == 0) // dPMR packet mode
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "+dPMRpkt     ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("+dPMRpkt   ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 21;
                return(23);
            }
        }
        if (m_opts.frame_dstar == 1)
        {
            if (strcmp(m_synctest, DSTAR_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "+D-STAR      ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +D-STAR   ",  m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 6;
                m_mbeRate = DSDMBERate3600x2400;
                return(6);
            }
            if (strcmp(m_synctest, INV_DSTAR_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "-D-STAR      ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" -D-STAR   ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 7;
                m_mbeRate = DSDMBERate3600x2400;
                return(7); // done
            }
            if (strcmp(m_synctest, DSTAR_HD) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "+D-STAR_HD   ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +D-STAR_HD   ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 18;
                m_mbeRate = DSDMBERate3600x2400;
                return(18); // done
            }
            if (strcmp(m_synctest, INV_DSTAR_HD) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "-D-STAR_HD   ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" -D-STAR_HD   ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = 19;
                m_mbeRate = DSDMBERate3600x2400;
                return(19);
            }
        }

        if ((m_t == 24) && (m_state.lastsynctype != -1))
        {
            if ((m_state.lastsynctype == 0)
                    && ((m_state.lastp25type == 1)
                            || (m_state.lastp25type == 2)))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "(+P25 Phase1)");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("(+P25p1)   ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = -1;
                m_mbeRate = DSDMBERate3600x2450;
                return(0); // done
            }
            else if ((m_state.lastsynctype == 1) && ((m_state.lastp25type == 1) || (m_state.lastp25type == 2)))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "(-P25 Phase1)");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("(-P25p1)   ",  m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = -1;
                m_mbeRate = DSDMBERate3600x2450;
                return(1); // done
            }
            else if ((m_state.lastsynctype == 3) && ((strcmp(m_synctest, X2TDMA_BS_VOICE_SYNC) != 0) || (strcmp(m_synctest, X2TDMA_MS_VOICE_SYNC) != 0)))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "(-X2-TDMA)   ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("(-X2-TDMA) ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = -1;
                m_mbeRate = DSDMBERate3600x2450;
                return(3);
            }
            else if ((m_state.lastsynctype == 4) && ((strcmp(m_synctest, X2TDMA_BS_DATA_SYNC) != 0) || (strcmp(m_synctest, X2TDMA_MS_DATA_SYNC) != 0)))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "(+X2-TDMA)   ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("(+X2-TDMA) ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = -1;
                m_mbeRate = DSDMBERate3600x2450;
                return(4);
            }
            else if ((m_state.lastsynctype == 11)  && ((strcmp(m_synctest, DMR_BS_VOICE_SYNC) != 0) || (strcmp(m_synctest, DMR_MS_VOICE_SYNC) != 0)))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "(-DMR)       ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("(-DMR)     ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = -1;
                m_mbeRate = DSDMBERate3600x2450;
                return(11); // done
            }
            else if ((m_state.lastsynctype == 12) && ((strcmp(m_synctest, DMR_BS_DATA_SYNC) != 0) || (strcmp(m_synctest, DMR_MS_DATA_SYNC) != 0)))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.snapSync(24);

                sprintf(m_state.ftype, "(+DMR)       ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("(+DMR)     ", m_synctest_pos + 1, m_modulation);
                }

                m_state.lastsynctype = -1;
                m_mbeRate = DSDMBERate3600x2450;
                return(12); // done
            }
        }
    }

    if (m_synctest_pos < 10200)
    {
        m_synctest_pos++;
        m_synctest_p++;
    }
    else
    {
        // buffer reset
        m_synctest_pos = 0;
        m_synctest_p = m_synctest_buf;
        noCarrier();
    }

    if (m_state.lastsynctype != 1)
    {
        if (m_synctest_pos >= 1800)
        {
            if ((m_opts.errorbars == 1) && (m_opts.verbose > 1)
                    && (m_state.carrier == 1))
            {
                m_dsdLogger.log("Sync: no sync\n");
            }

            sprintf(m_state.ftype, "No Sync      ");
            noCarrier();
            return(-1); // done
        }
    }

    return(-2); // still searching
}

void DSDDecoder::resetFrameSync()
{
    m_dsdLogger.log("DSDDecoder::resetFrameSync: symbol %d (%d)\n", m_state.symbolcnt, m_dsdSymbol.getSymbol());

    m_dsdSymbol.resetFrameSync();

    // reset detect frame sync engine
    m_t = 0;
    m_synctest[24] = 0;
    m_synctest18[18] = 0;
    m_synctest32[32] = 0;
    m_synctest_pos = 0;
    m_synctest_p = m_synctest_buf + 10;
    m_state.numflips = 0;

    m_sync = -2;   // mark in progress
    m_hasSync = false; // for DSDSymbol::pushSample method

    if ((m_opts.symboltiming == 1) && (m_state.carrier == 1))
    {
        m_dsdLogger.log("\nSymbol Timing:\n");
    }

    m_fsmState = DSDLookForSync;
}

void DSDDecoder::printFrameSync(const char *frametype, int offset, char *modulation)
{
    if (m_opts.verbose > 0)
    {
        m_dsdLogger.log("Sync: %s ", frametype);
    }
    if (m_opts.verbose > 2)
    {
        m_dsdLogger.log("o: %4i ", offset);
    }
    if (m_opts.verbose > 1)
    {
        m_dsdLogger.log("mod: %s ", modulation);
    }
    if (m_opts.verbose > 2)
    {
        m_dsdLogger.log("g: %f ", m_state.aout_gain);
    }
}

void DSDDecoder::noCarrier()
{
    m_state.dibit_buf_p = m_state.dibit_buf + 200;
    memset(m_state.dibit_buf, 0, sizeof(int) * 200);

    m_dsdSymbol.noCarrier();

    m_state.lastsynctype = -1;
    m_state.carrier = 0;
    m_state.max = 15000;
    m_state.min = -15000;
    m_state.center = 0;
    m_state.err_str[0] = 0;

    sprintf(m_state.fsubtype, "              ");
    sprintf(m_state.ftype, "             ");

    m_state.errs = 0;
    m_state.errs2 = 0;
    m_state.lasttg = 0;
    m_state.lastsrc = 0;
    m_state.lastp25type = 0;
    m_state.repeat = 0;
    m_state.nac = 0;
    m_state.numtdulc = 0;

    sprintf(m_state.slot0light, " slot0 ");
    sprintf(m_state.slot1light, " slot1 ");

    m_state.firstframe = 0;

    if (m_opts.audio_gain == (float) 0)
    {
        m_state.aout_gain = 25;
    }

    memset(m_state.aout_max_buf, 0, sizeof(float) * 200);
    m_state.aout_max_buf_p = m_state.aout_max_buf;
    m_state.aout_max_buf_idx = 0;
    sprintf(m_state.algid, "________");
    sprintf(m_state.keyid, "________________");
    mbe_initMbeParms(m_state.cur_mp, m_state.prev_mp, m_state.prev_mp_enhanced);
}

void DSDDecoder::printFrameInfo()
{

    int level = (int) m_state.max / 164;

    if (m_opts.verbose > 0)
    {
        m_dsdLogger.log("inlvl: %2i%% ", level);
    }
    if (m_state.nac != 0)
    {
        m_dsdLogger.log("nac: %4X ", m_state.nac);
    }

    if (m_opts.verbose > 1)
    {
        m_dsdLogger.log("src: %8i ", m_state.lastsrc);
    }

    m_dsdLogger.log("tg: %5i ", m_state.lasttg);
}

int DSDDecoder::comp(const void *a, const void *b)
{
    if (*((const int *) a) == *((const int *) b))
        return 0;
    else if (*((const int *) a) < *((const int *) b))
        return -1;
    else
        return 1;
}

} // namespace dsdcc
