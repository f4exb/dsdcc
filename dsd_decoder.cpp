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
        m_mbeDVReady1(false),
        m_mbelibEnable(true),
        m_mbeRate(DSDMBERateNone),
        m_dsdSymbol(this),
        m_mbeDecoder1(this),
        m_mbeDecoder2(this),
        m_dsdDMRVoice(this),
        m_dsdDMRData(this),
        m_dsdDMR(this),
        m_dsdDstar(this),
        m_dsdYSF(this),
        m_dsdDPMR(this),
		m_dsdNXDN(this),
        m_dataRate(DSDRate4800),
        m_syncType(DSDSyncNone),
        m_lastSyncType(DSDSyncNone)
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

void DSDDecoder::setAudioGain(float gain)
{
    m_opts.audio_gain = gain;

    if (m_opts.audio_gain < 0.0f)
    {
        m_dsdLogger.log("Audio out gain invalid\n");
    }
    else if (m_opts.audio_gain == 0.0f)
    {
        m_dsdLogger.log("Enabling audio out auto-gain\n");
    	m_mbeDecoder1.setAudioGain(25);
    	m_mbeDecoder1.setAutoGain(true);
        m_mbeDecoder2.setAudioGain(25);
        m_mbeDecoder2.setAutoGain(true);
    }
    else
    {
        m_dsdLogger.log("Setting audio out gain to %f\n", m_opts.audio_gain);
        m_mbeDecoder1.setAudioGain(m_opts.audio_gain);
        m_mbeDecoder1.setAutoGain(false);
        m_mbeDecoder2.setAudioGain(m_opts.audio_gain);
        m_mbeDecoder2.setAutoGain(false);
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
    if ((upsampling != 6) && (upsampling != 7)) {
    	upsampling = 0;
    }

    m_mbeDecoder1.setUpsamplingFactor(upsampling);
    m_mbeDecoder2.setUpsamplingFactor(upsampling);
    m_dsdLogger.log("Setting upsampling to x%d\n", (upsampling == 0 ? 1 : upsampling));
}

void DSDDecoder::setStereo(bool on)
{
	m_mbeDecoder1.setStereo(on);
	m_mbeDecoder2.setStereo(on);
}

void DSDDecoder::setInvertedXTDMA(bool on)
{
    m_opts.inverted_x2tdma = (on ? 1 : 0);
    m_dsdLogger.log("Expecting %sinverted X2-TDMA signals.\n", (m_opts.inverted_x2tdma == 0 ? "non-" : ""));
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
        m_dsdSymbol.setSamplesPerSymbol(20);
        break;
    case DSDRate4800:
        m_dsdLogger.log("Set data rate to 4800 bauds. 10 samples per symbol\n");
        m_dsdSymbol.setSamplesPerSymbol(10);
        break;
    case DSDRate9600:
        m_dsdLogger.log("Set data rate to 9600 bauds. 5 samples per symbol\n");
        m_dsdSymbol.setSamplesPerSymbol(5);
        break;
    default:
        m_dsdLogger.log("Set default data rate to 4800 bauds. 10 samples per symbol\n");
        m_dsdSymbol.setSamplesPerSymbol(10);
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

    if (m_dsdSymbol.pushSample(sample)) // a symbol is retrieved
    {
        switch (m_fsmState)
        {
        case DSDLookForSync:
            m_sync = getFrameSync(); // -> -2: still looking, -1 not found, 0 and above: sync found

            if (m_sync == -2) // -2 means no sync has been found at all
            {
                break; // still searching -> no change in FSM state
            }
            else if (m_sync == -1) // -1 means sync has been found but is invalid
            {
                m_dsdLogger.log("DSDDecoder::run: invalid sync found: %d symbol %d (%d)\n", m_sync, m_state.symbolcnt, m_dsdSymbol.getSymbol());
                resetFrameSync(); // go back searching
            }
            else // good sync found
            {
                m_dsdLogger.log("DSDDecoder::run: good sync found: %d symbol %d (%d)\n", m_sync, m_state.symbolcnt, m_dsdSymbol.getSymbol());
                m_fsmState = DSDSyncFound; // go to processing state next time
            }

            break; // next
        case DSDSyncFound:
            m_syncType  = (DSDSyncType) m_sync;
            m_dsdLogger.log("DSDDecoder::run: before processFrameInit: symbol %d (%d)\n", m_state.symbolcnt, m_dsdSymbol.getSymbol());
            processFrameInit();   // initiate the process of the frame which sync has been found. This will change FSM state
            break;
        case DSDprocessDMRvoice:
            m_dsdDMR.processVoice();
//            m_dsdDMRVoice.process();
            break;
        case DSDprocessDMRdata:
            m_dsdDMR.processData();
//            m_dsdDMRData.process();
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
    if ((m_syncType == DSDSyncDMRDataP)
            || (m_syncType == DSDSyncDMRVoiceN)
            || (m_syncType == DSDSyncDMRVoiceP)
            || (m_syncType == DSDSyncDMRDataN)) // DMR
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = m_dsdSymbol.getLevel();
                m_dsdLogger.log("inlvl: %2i%% ", level);
            }
        }

        if ((m_syncType == DSDSyncDMRVoiceN) || (m_syncType == DSDSyncDMRVoiceP))
        {
            sprintf(m_state.fsubtype, " VOICE        ");
            m_dsdDMR.initVoice(m_dmrBurstType);    // initializations not consuming a live symbol
            m_dsdDMR.processVoice(); // process current symbol first
//            m_dsdDMRVoice.init();    // initializations not consuming a live symbol
//            m_dsdDMRVoice.process(); // process current symbol first
            m_fsmState = DSDprocessDMRvoice;
        }
        else
        {
            m_dsdDMR.initData(m_dmrBurstType);    // initializations not consuming a live symbol
            m_dsdDMR.processData(); // process current symbol first
//            m_dsdDMRData.init();    // initializations not consuming a live symbol
//            m_dsdDMRData.process(); // process current symbol first
            m_fsmState = DSDprocessDMRdata;
        }
    }
    else if ((m_syncType == DSDSyncDStarP) || (m_syncType == DSDSyncDStarN)) // D-Star voice
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = m_dsdSymbol.getLevel();
                printf("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " VOICE        ");
        m_dsdDstar.init();
        m_dsdDstar.process(); // process current symbol first
        m_fsmState = DSDprocessDSTAR;
    }
    else if ((m_syncType == DSDSyncNXDNP) || (m_syncType == DSDSyncNXDNN)) // NXDN conventional
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = m_dsdSymbol.getLevel();
                printf("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " RDCH         ");
        m_dsdNXDN.init();
        m_dsdNXDN.process(); // process current symbol first
        m_fsmState = DSDprocessNXDN;
    }
    else if ((m_syncType == DSDSyncDStarHeaderP) || (m_syncType == DSDSyncDStarHeaderN)) // D-Star header
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = m_dsdSymbol.getLevel();
                m_dsdLogger.log("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " DATA         ");
        m_dsdDstar.init(true);
        m_dsdDstar.processHD(); // process current symbol first
        m_fsmState = DSDprocessDSTAR_HD;
    }
    else if (m_syncType == DSDSyncDPMR) // dPMR classic (not packet)
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = m_dsdSymbol.getLevel();
                m_dsdLogger.log("inlvl: %2i%% ", level);
            }
        }

        m_state.nac = 0;
        sprintf(m_state.fsubtype, " ANY          ");
        m_dsdDPMR.init();
        m_dsdDPMR.process();
        m_fsmState = DSDprocessDPMR;
    }
    else if (m_syncType == DSDSyncYSF) // YSF
    {
        m_state.nac = 0;
        m_state.lastsrc = 0;
        m_state.lasttg = 0;

        if (m_opts.errorbars == 1)
        {
            if (m_opts.verbose > 0)
            {
                int level = m_dsdSymbol.getLevel();
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
     * integer values mapping DSDSyncType enum:
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

    if (m_state.dibit_buf_p > m_state.dibit_buf + 900000)
    {
        m_state.dibit_buf_p = m_state.dibit_buf + 200;
    }

    //determine dibit state
    if (m_dsdSymbol.getSymbol() > 0)
    {
        m_dibit = 49; // ASCII character '1'
    }
    else
    {
        m_dibit = 51; // ASCII character '3'
    }

    *m_synctest_p = m_dibit;

    if (m_t < 18)
    {
        m_t++;
    }
    else
    {
        // Sync identification starts here

        strncpy(m_synctest, (m_synctest_p - 23), 24);
        m_stationType = DSDStationTypeNotApplicable;
        m_dmrBurstType = DSDDMR::DSDDMRBurstNone;

        if (m_opts.frame_p25p1 == 1)
        {
            if (strcmp(m_synctest, P25P1_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4);

                sprintf(m_state.ftype, "+P25 Phase 1 ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +P25p1    ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncP25p1P;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncP25p1P;
            }
            if (strcmp(m_synctest, INV_P25P1_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4, true);

                sprintf(m_state.ftype, "-P25 Phase 1 ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" -P25p1    ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncP25p1N;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncP25p1N;
            }
        }
        if (m_opts.frame_x2tdma == 1)
        {
            if ((strcmp(m_synctest, X2TDMA_BS_DATA_SYNC) == 0)
             || (strcmp(m_synctest, X2TDMA_MS_DATA_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4);

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
                        printFrameSync(" +X2-TDMA  ", m_synctest_pos + 1);
                    }

                    m_lastSyncType = DSDSyncX2TDMADataP;
                    m_mbeRate = DSDMBERate3600x2450;
                    return (int) DSDSyncX2TDMADataP; // done
                }
                else
                {
                    // inverted voice frame
                    sprintf(m_state.ftype, "-X2-TDMAv    ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" -X2-TDMA  ", m_synctest_pos + 1);
                    }

                    if (m_lastSyncType != DSDSyncX2TDMAVoiceN)
                    {
                        m_state.firstframe = 1;
                    }

                    m_lastSyncType = DSDSyncX2TDMAVoiceN;
                    m_mbeRate = DSDMBERate3600x2450;
                    return (int) DSDSyncX2TDMAVoiceN; // done
                }
            }
            if ((strcmp(m_synctest, X2TDMA_BS_VOICE_SYNC) == 0)
             || (strcmp(m_synctest, X2TDMA_MS_VOICE_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4);

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
                        printFrameSync(" +X2-TDMA  ", m_synctest_pos + 1);
                    }

                    if (m_lastSyncType != DSDSyncX2TDMAVoiceP)
                    {
                        m_state.firstframe = 1;
                    }

                    m_lastSyncType = DSDSyncX2TDMAVoiceP;
                    m_mbeRate = DSDMBERate3600x2450;
                    return (int) DSDSyncX2TDMAVoiceP; // done
                }
                else
                {
                    // inverted data frame
                    sprintf(m_state.ftype, "-X2-TDMAd    ");

                    if (m_opts.errorbars == 1)
                    {
                        printFrameSync(" -X2-TDMA  ", m_synctest_pos + 1);
                    }

                    m_lastSyncType = DSDSyncX2TDMADataN;
                    m_mbeRate = DSDMBERate3600x2450;
                    return (int) DSDSyncX2TDMADataN; // done
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
                m_dsdSymbol.setFSK(4);

                sprintf(m_state.ftype, "+YSF         ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("+YSF       ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncYSF;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncYSF;
            }
        }
        if (m_opts.frame_dmr == 1)
        {
            if ((strcmp(m_synctest, DMR_MS_DATA_SYNC) == 0)
             || (strcmp(m_synctest, DMR_BS_DATA_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4);

                if (strcmp(m_synctest, DMR_BS_DATA_SYNC) == 0)
                {
                    m_stationType = DSDBaseStation;
                    m_dmrBurstType = DSDDMR::DSDDMRBaseStation;
                }
                else
                {
                    m_stationType = DSDMobileStation;
                    m_dmrBurstType = DSDDMR::DSDDMRMobileStation;
                }

				// data frame
				sprintf(m_state.ftype, "+DMRd        ");

				if (m_opts.errorbars == 1)
				{
					printFrameSync(" +DMRd     ",  m_synctest_pos + 1);
				}

				m_lastSyncType = DSDSyncDMRDataP;
				m_mbeRate = DSDMBERate3600x2450;
				return (int) DSDSyncDMRDataP; // done
            }
            if ((strcmp(m_synctest, DMR_MS_VOICE_SYNC) == 0)
             || (strcmp(m_synctest, DMR_BS_VOICE_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4);

                if (strcmp(m_synctest, DMR_BS_VOICE_SYNC) == 0)
                {
                    m_stationType = DSDBaseStation;
                    m_dmrBurstType = DSDDMR::DSDDMRBaseStation;
                }
                else
                {
                    m_stationType = DSDMobileStation;
                    m_dmrBurstType = DSDDMR::DSDDMRMobileStation;
                }

				// voice frame
				sprintf(m_state.ftype, "+DMRv        ");

				if (m_opts.errorbars == 1)
				{
					printFrameSync(" +DMRv     ", m_synctest_pos + 1);
				}

				if (m_lastSyncType != DSDSyncDMRVoiceP)
				{
					m_state.firstframe = 1;
				}

				m_lastSyncType = DSDSyncDMRVoiceP;
				m_mbeRate = DSDMBERate3600x2450;
				return (int) DSDSyncDMRVoiceP; // done
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
                m_dsdSymbol.setFSK(4);

                sprintf(m_state.ftype, "+ProVoice    ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +ProVoice ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncProVoiceP;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncProVoiceP; // done
            }
            else if ((strcmp(m_synctest32, INV_PROVOICE_SYNC) == 0)
                  || (strcmp(m_synctest32, INV_PROVOICE_EA_SYNC) == 0))
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4, true);

                sprintf(m_state.ftype, "-ProVoice    ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" -ProVoice ",  m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncProVoiceN;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncProVoiceN; // donesynctype
            }

        }
        if ((m_opts.frame_nxdn96 == 1) || (m_opts.frame_nxdn48 == 1))
        {
        	strncpy(m_synctest20, (m_synctest_p - 19), 20);

            if (strcmp(m_synctest20, NXDN_RDCH_FULL_SYNC) == 0)
            {
				m_state.carrier = 1;
				m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4);

                if (m_dataRate == DSDRate2400)
				{
					sprintf(m_state.ftype, "+NXDN48      ");

					if (m_opts.errorbars == 1)
					{
						printFrameSync(" +NXDN48   ", m_synctest_pos + 1);
					}
				}
				else
				{
					sprintf(m_state.ftype, "+NXDN96      ");

					if (m_opts.errorbars == 1)
					{
						printFrameSync(" +NXDN96   ", m_synctest_pos + 1);
					}
				}

				m_lastSyncType = DSDSyncNXDNP;
				m_mbeRate = DSDMBERate3600x2450;
				return (int) DSDSyncNXDNP; // done
            }
            else if (strcmp(m_synctest20, INV_NXDN_RDCH_FULL_SYNC) == 0)
            {
				m_state.carrier = 1;
				m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4, true);

                if (m_dataRate == DSDRate2400)
				{
					sprintf(m_state.ftype, "-NXDN48      ");

					if (m_opts.errorbars == 1)
					{
						printFrameSync(" -NXDN48   ", m_synctest_pos + 1);
					}
				}
				else
				{
					sprintf(m_state.ftype, "-NXDN96      ");

					if (m_opts.errorbars == 1)
					{
						printFrameSync(" -NXDN96   ", m_synctest_pos + 1);
					}
				}

                m_lastSyncType = DSDSyncNXDNN;
				m_mbeRate = DSDMBERate3600x2450;
				return (int) DSDSyncNXDNN; // done
            }
        }
        if (m_opts.frame_dpmr == 1)
        {
            if (strcmp(m_synctest, DPMR_FS1_SYNC) == 0) // dPMR classic (not packet)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4);

                sprintf(m_state.ftype, "+dPMR        ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("+dPMR      ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncDPMR;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncDPMR;
            }
            else if (strcmp(m_synctest, DPMR_FS4_SYNC) == 0) // dPMR packet mode
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(4);

                sprintf(m_state.ftype, "+dPMRpkt     ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("+dPMRpkt   ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncDPMRPacket;
                return (int) DSDSyncDPMRPacket;
            }
        }
        if (m_opts.frame_dstar == 1)
        {
            if (strcmp(m_synctest, DSTAR_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(2);

                sprintf(m_state.ftype, "+D-STAR      ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +D-STAR   ",  m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncDStarP;
                m_mbeRate = DSDMBERate3600x2400;
                return (int) DSDSyncDStarP;
            }
            if (strcmp(m_synctest, INV_DSTAR_SYNC) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(2, true);

                sprintf(m_state.ftype, "-D-STAR      ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" -D-STAR   ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncDStarN;
                m_mbeRate = DSDMBERate3600x2400;
                return (int) DSDSyncDStarN; // done
            }
            if (strcmp(m_synctest, DSTAR_HD) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(2);

                sprintf(m_state.ftype, "+D-STAR_HD   ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +D-STAR_HD   ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncDStarHeaderP;
                m_mbeRate = DSDMBERate3600x2400;
                return (int) DSDSyncDStarHeaderP; // done
            }
            if (strcmp(m_synctest, INV_DSTAR_HD) == 0)
            {
                m_state.carrier = 1;
                m_state.offset = m_synctest_pos;
                m_dsdSymbol.setFSK(2, true);

                sprintf(m_state.ftype, "-D-STAR_HD   ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" -D-STAR_HD   ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncDStarHeaderN;
                m_mbeRate = DSDMBERate3600x2400;
                return (int) DSDSyncDStarHeaderN;
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

    // if (m_state.lastsynctype != 1) ... test removed
    // {

    if (m_synctest_pos >= 1800)
    {
        if ((m_opts.errorbars == 1) && (m_opts.verbose > 1)
                && (m_state.carrier == 1))
        {
            m_dsdLogger.log("Sync: no sync\n");
        }

        sprintf(m_state.ftype, "No Sync      ");
        noCarrier();
        return -1; // done
    }

    // }

    return -2; // still searching
}

void DSDDecoder::resetFrameSync()
{
    m_dsdLogger.log("DSDDecoder::resetFrameSync: symbol %d (%d)\n", m_state.symbolcnt, m_dsdSymbol.getSymbol());

//    m_dsdSymbol.resetFrameSync();

    // reset detect frame sync engine
    m_t = 0;
    m_synctest[24] = 0;
    m_synctest18[18] = 0;
    m_synctest32[32] = 0;
    m_synctest_pos = 0;
    m_synctest_p = m_synctest_buf + 10;

    m_sync = -2;   // mark in progress

    if ((m_opts.symboltiming == 1) && (m_state.carrier == 1))
    {
        m_dsdLogger.log("\nSymbol Timing:\n");
    }

    m_fsmState = DSDLookForSync;
}

void DSDDecoder::printFrameSync(const char *frametype, int offset)
{
    if (m_opts.verbose > 0)
    {
        m_dsdLogger.log("Sync: %s ", frametype);
    }
    if (m_opts.verbose > 2)
    {
        m_dsdLogger.log("o: %4i ", offset);
    }
}

void DSDDecoder::noCarrier()
{
    m_state.dibit_buf_p = m_state.dibit_buf + 200;
    memset(m_state.dibit_buf, 0, sizeof(int) * 200);

    m_dsdSymbol.noCarrier();

    m_lastSyncType = DSDSyncNone;
    m_state.carrier = 0;

    sprintf(m_state.fsubtype, "              ");
    sprintf(m_state.ftype, "             ");
    m_state.fsubtype[0] = '\0';
    m_state.ftype[0] = '\0';

    m_state.lasttg = 0;
    m_state.lastsrc = 0;
    m_state.lastp25type = 0;
    m_state.repeat = 0;
    m_state.nac = 0;
    m_state.numtdulc = 0;

//    sprintf(m_state.slot0light, " slot0 ");
//    sprintf(m_state.slot1light, " slot1 ");

    m_state.firstframe = 0;

    sprintf(m_state.algid, "________");
    sprintf(m_state.keyid, "________________");
    m_mbeDecoder1.initMbeParms();
    m_mbeDecoder2.initMbeParms();
}

void DSDDecoder::printFrameInfo()
{

    int level = m_dsdSymbol.getLevel();

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
