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
#include "timeutil.h"
#include "dsd_decoder.h"

namespace DSDcc
{

// Sync wods - Symbol mapping: 01(1):+3, 00(0):+1, 10(2):-1, 11(3):-3
const unsigned char DSDDecoder::m_syncDMRDataBS[24]       = {3, 1, 3, 3, 3, 3, 1, 1, 1, 3, 3, 1, 1, 3, 1, 1, 3, 1, 3, 3, 1, 1, 3, 1}; // DF F5 7D 75 DF 5D
const unsigned char DSDDecoder::m_syncDMRVoiceBS[24]      = {1, 3, 1, 1, 1, 1, 3, 3, 3, 1, 1, 3, 3, 1, 3, 3, 1, 3, 1, 1, 3, 3, 1, 3}; // 75 5F D7 DF 75 F7
const unsigned char DSDDecoder::m_syncDMRDataMS[24]       = {3, 1, 1, 1, 3, 1, 1, 3, 3, 3, 1, 3, 1, 3, 3, 3, 3, 1, 1, 3, 1, 1, 1, 3}; // D5 D7 F7 7F D7 57
const unsigned char DSDDecoder::m_syncDMRVoiceMS[24]      = {1, 3, 3, 3, 1, 3, 3, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 3, 3, 1, 3, 3, 3, 1}; // 7F 7D 5D D5 7D FD
const unsigned char DSDDecoder::m_syncDPMRFS1[24]         = {1, 1, 1, 3, 3, 3, 3, 3, 1, 1, 3, 3, 1, 3, 1, 1, 3, 1, 1, 1, 1, 3, 1, 3}; // 57 FF 5F 75 D5 77 - non packet data header
const unsigned char DSDDecoder::m_syncDPMRFS4[24]         = {3, 3, 3, 1, 1, 1, 1, 1, 3, 3, 1, 1, 3, 1, 3, 3, 1, 3, 3, 3, 3, 1, 3, 1}; // FD 55 F5 DF 7F DD - packet data header
const unsigned char DSDDecoder::m_syncDPMRFS2[12]         = {1, 1, 3, 3, 3, 3, 1, 3, 1, 3, 3, 1};                                     // 5F F7 7D          - superframe sync (each 2 384 bit frames)
const unsigned char DSDDecoder::m_syncDPMRFS3[12]         = {1, 3, 3, 1, 3, 1, 3, 3, 3, 3, 1, 1};                                     // 7D DF F5           end frame sync
const unsigned char DSDDecoder::m_syncNXDNRDCHFull[19]    = {3, 1, 3, 1, 1, 3, 3, 3, 1, 3, 1, 3, 1, 3, 3, 1, 1, 3, 1};
const unsigned char DSDDecoder::m_syncNXDNRDCHFullInv[19] = {1, 3, 1, 3, 3, 1, 1, 1, 3, 1, 3, 1, 3, 1, 1, 3, 3, 1, 3};
//const unsigned char DSDDecoder::m_syncNXDNRDCHFull[20]    = {1, 3, 1, 3, 1, 1, 3, 3, 3, 1, 3, 1, 3, 1, 3, 3, 1, 1, 3, 1};
//const unsigned char DSDDecoder::m_syncNXDNRDCHFullInv[20] = {3, 1, 3, 1, 3, 3, 1, 1, 1, 3, 1, 3, 1, 3, 1, 1, 3, 3, 1, 3};
const unsigned char DSDDecoder::m_syncNXDNRDCHFSW[10]     = {3, 1, 3, 1, 3, 3, 1, 1, 3, 1};
const unsigned char DSDDecoder::m_syncNXDNRDCHFSWInv[10]  = {1, 3, 1, 3, 1, 1, 3, 3, 1, 3};
const unsigned char DSDDecoder::m_syncDStarHeader[24]     = {1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 3, 3, 1, 3, 3, 1, 1, 3, 1, 3, 1, 1, 1, 1};
const unsigned char DSDDecoder::m_syncDStarHeaderInv[24]  = {3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 1, 1, 3, 1, 1, 3, 3, 1, 3, 1, 3, 3, 3, 3};
const unsigned char DSDDecoder::m_syncDStar[24]           = {3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 3, 1, 3, 1, 1, 1, 3, 3, 1, 3, 1, 1, 1};
const unsigned char DSDDecoder::m_syncDStarInv[24]        = {1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 1, 3, 1, 3, 3, 3, 1, 1, 3, 1, 3, 3, 3};
const unsigned char DSDDecoder::m_syncYSF[20]             = {3, 1, 1, 1, 1, 3, 1, 1, 3, 1, 3, 1, 1, 3, 1, 3, 1, 1, 3, 1}; // D4 71 C9 63 4D => D5 75 DD 77 5D
const unsigned char DSDDecoder::m_syncP25P1[24]           = {1, 1, 1, 1, 1, 3, 1, 1, 3, 3, 1, 1, 3, 3, 3, 3, 1, 3, 1, 3, 3, 3, 3, 3};
const unsigned char DSDDecoder::m_syncP25P1Inv[24]        = {3, 3, 3, 3, 3, 1, 3, 3, 1, 1, 3, 3, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1};
const unsigned char DSDDecoder::m_syncX2TDMADataBS[24]    = {3, 3, 1, 3, 1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 3, 3, 3, 1, 1, 3, 3};
const unsigned char DSDDecoder::m_syncX2TDMAVoiceBS[24]   = {1, 1, 3, 1, 3, 1, 3, 3, 3, 3, 3, 1, 3, 1, 3, 3, 3, 1, 1, 1, 3, 3, 1, 1};
const unsigned char DSDDecoder::m_syncX2TDMADataMS[24]    = {3, 1, 3, 1, 1, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3 ,3 ,3 ,1 ,3};
const unsigned char DSDDecoder::m_syncX2TDMAVoiceMS[24]   = {1, 3, 1, 3, 3, 1, 1, 1, 1, 3 ,3, 3, 3, 3, 3, 3, 1, 1, 1, 1 ,1 ,1 ,3 ,1};
const unsigned char DSDDecoder::m_syncProVoice[32]        = {1, 3, 1, 3, 1, 3, 3, 3, 1, 1, 1, 3, 1, 1, 3, 1, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1, 3, 3};
const unsigned char DSDDecoder::m_syncProVoiceInv[32]     = {3, 1, 3, 1, 3, 1, 1, 1, 3, 3, 3, 1, 3, 3, 1, 3, 3, 3, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1};
const unsigned char DSDDecoder::m_syncProVoiceEA[32]      = {3, 1, 1, 3, 1, 3, 1, 1, 3, 3, 1, 3, 3, 1, 1, 1, 1, 1, 3, 3, 1, 3, 1, 3, 1, 1, 3 ,1 ,1 ,1, 3 ,3};
const unsigned char DSDDecoder::m_syncProVoiceEAInv[32]   = {1, 3, 3, 1, 3, 1, 3, 3, 1, 1, 3, 1, 1 ,3 ,3, 3, 3 ,3, 1, 1 ,3, 1, 3, 1, 3, 3, 1, 3, 3, 3, 1, 1};


DSDDecoder::DSDDecoder() :
        m_fsmState(DSDLookForSync),
        m_dsdSymbol(this),
        m_mbelibEnable(true),
        m_mbeRate(DSDMBERateNone),
        m_mbeDecoder1(this),
        m_mbeDecoder2(this),
        m_mbeDVReady1(false),
        m_dsdDMR(this),
        m_dsdDstar(this),
        m_dsdYSF(this),
        m_dsdDPMR(this),
		m_dsdNXDN(this),
        m_dataRate(DSDRate4800),
        m_syncType(DSDSyncNone),
        m_lastSyncType(DSDSyncNone),
        m_signalFormat(signalFormatNone)
{
    resetFrameSync();
    noCarrier();
    m_squelchTimeoutCount = 0;
    m_nxdnInterSyncCount = -1; // reset to quiet state
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

void DSDDecoder::showErrorBars()
{
    m_opts.errorbars = 1;
}

void DSDDecoder::showSymbolTiming()
{
    m_opts.symboltiming = 1;
    m_opts.errorbars = 1;
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

void DSDDecoder::setDMRBasicPrivacyKey(unsigned char key)
{
    m_opts.dmr_bp_key = key;
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

    resetFrameSync();
    noCarrier();
    m_squelchTimeoutCount = 0;
    m_nxdnInterSyncCount = -1; // reset to quiet state
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
        m_mbeDecoder1.setVolume(1.0f);
        m_mbeDecoder1.setAutoGain(true);
        m_mbeDecoder2.setAudioGain(25);
        m_mbeDecoder2.setVolume(1.0f);
        m_mbeDecoder2.setAutoGain(true);
    }
    else
    {
        m_dsdLogger.log("Setting audio out gain to %f\n", m_opts.audio_gain);
        m_mbeDecoder1.setAudioGain(m_opts.audio_gain);
        m_mbeDecoder1.setVolume(m_opts.audio_gain);
        m_mbeDecoder1.setAutoGain(false);
        m_mbeDecoder2.setAudioGain(m_opts.audio_gain);
        m_mbeDecoder2.setVolume(m_opts.audio_gain);
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
    if (upsampling > 7) {
        upsampling = 7;
    }

    if (upsampling < 0) { // 0 or 1 means 1
        upsampling = 0;
    }

    m_mbeDecoder1.setUpsamplingFactor(upsampling);
    m_mbeDecoder2.setUpsamplingFactor(upsampling);
    m_dsdLogger.log("Setting upsampling to x%d\n", upsampling);
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
        else
        {
            m_squelchTimeoutCount = 0;
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
            break;
        case DSDprocessDMRvoiceMS:
            m_dsdDMR.processVoiceMS();
            break;
        case DSDprocessDMRdata:
            m_dsdDMR.processData();
            break;
        case DSDprocessDMRdataMS:
            m_dsdDMR.processDataMS();
            break;
        case DSDprocessDMRsyncOrSkip:
            m_dsdDMR.processSyncOrSkip();
            break;
        case DSDprocessDMRSkipMS:
            m_dsdDMR.processSkipMS();
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
            || (m_syncType == DSDSyncDMRVoiceP)) // DMR
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

        if (m_syncType == DSDSyncDMRVoiceP)
        {
            sprintf(m_state.fsubtype, " VOICE        ");
            m_dsdDMR.initVoice();    // initializations not consuming a live symbol
            m_dsdDMR.processVoice(); // process current symbol first
            m_fsmState = DSDprocessDMRvoice;
        }
        else
        {
            m_dsdDMR.initData();    // initializations not consuming a live symbol
            m_dsdDMR.processData(); // process current symbol first
            m_fsmState = DSDprocessDMRdata;
        }
    }
    else if ((m_syncType == DSDSyncDMRDataMS) || (m_syncType == DSDSyncDMRVoiceMS))
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

        if (m_syncType == DSDSyncDMRVoiceMS)
        {
            sprintf(m_state.fsubtype, " VOICE        ");
            m_dsdDMR.initVoiceMS();    // initializations not consuming a live symbol
            m_dsdDMR.processVoiceMS(); // process current symbol first
            m_fsmState = DSDprocessDMRvoiceMS;
        }
        else
        {
            m_dsdDMR.initDataMS();    // initializations not consuming a live symbol
            m_dsdDMR.processDataMS(); // process current symbol first
            m_fsmState = DSDprocessDMRdataMS;
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
    else if ((m_syncType == DSDSyncNXDNP) || (m_syncType == DSDSyncNXDNN)) // NXDN full sync with preamble
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
     * 8  = +NXDN (non inverted FCH)
     * 9  = -NXDN (inverted FCH)
     * 10 = +DMR (non inverted signal data frame)
     * 11 = +DMR (non inverted signal data frame for mobile station)
     * 12 = +DMR (non inverted signal voice frame)
     * 13 = +DMR (non inverted signal voice frame for mobile station)
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

    if (m_t < 18)
    {
        m_t++;
    }
    else // Sync identification starts here
    {
        m_dmrBurstType = DSDDMR::DSDDMRBurstNone;

        if (m_opts.frame_p25p1 == 1)
        {
            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncP25P1, 24) == 0)
            {
                m_state.carrier = 1;
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
            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncP25P1Inv, 24) == 0)
            {
                m_state.carrier = 1;
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
            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncX2TDMADataBS, 24) == 0)
            {
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);
                m_stationType = DSDBaseStation;

                sprintf(m_state.ftype, "+X2-TDMAd    ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +X2-TDMA  ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncX2TDMADataP;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncX2TDMADataP; // done
            }

            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncX2TDMADataMS, 24) == 0)
            {
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);
                m_stationType = DSDMobileStation;

                sprintf(m_state.ftype, "+X2-TDMAd    ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +X2-TDMA  ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncX2TDMADataP;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncX2TDMADataP; // done
            }

            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncX2TDMAVoiceBS, 24) == 0)
            {
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);
                m_stationType = DSDBaseStation;

                sprintf(m_state.ftype, "+X2-TDMAv    ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +X2-TDMA  ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncX2TDMAVoiceP;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncX2TDMAVoiceP; // done
            }

            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncX2TDMAVoiceMS, 24) == 0)
            {
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);
                m_stationType = DSDMobileStation;

                sprintf(m_state.ftype, "+X2-TDMAv    ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +X2-TDMA  ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncX2TDMAVoiceP;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncX2TDMAVoiceP; // done
            }
        }
        if (m_opts.frame_ysf == 1)
        {
            if (memcmp(m_dsdSymbol.getSyncDibitBack(20), m_syncYSF, 20) == 0)
            {
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);

                sprintf(m_state.ftype, "+YSF         ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync("+YSF       ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncYSF;
                m_mbeRate = DSDMBERateNone; // choice is done inside the YSF decoder between the 3 possible modes
                return (int) DSDSyncYSF;
            }
        }
        if (m_opts.frame_dmr == 1)
        {
        	if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDMRDataBS, 24) == 0)
        	{
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);

                m_stationType = DSDBaseStation;
                m_dmrBurstType = DSDDMR::DSDDMRBaseStation;

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

            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDMRDataMS, 24) == 0)
            {
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);

                m_stationType = DSDMobileStation;
                m_dmrBurstType = DSDDMR::DSDDMRMobileStation;

                // data frame
                sprintf(m_state.ftype, "+DMRd        ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +DMRd     ",  m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncDMRDataMS;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncDMRDataMS; // done
            }

        	if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDMRVoiceBS, 24) == 0)
        	{
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);

                m_stationType = DSDBaseStation;
                m_dmrBurstType = DSDDMR::DSDDMRBaseStation;

				// voice frame
				sprintf(m_state.ftype, "+DMRv        ");

				if (m_opts.errorbars == 1)
				{
					printFrameSync(" +DMRv     ", m_synctest_pos + 1);
				}

				m_lastSyncType = DSDSyncDMRVoiceP;
				m_mbeRate = DSDMBERate3600x2450;
				return (int) DSDSyncDMRVoiceP; // done
        	}

            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDMRVoiceMS, 24) == 0)
            {
                m_state.carrier = 1;
                m_dsdSymbol.setFSK(4);

                m_stationType = DSDMobileStation;
                m_dmrBurstType = DSDDMR::DSDDMRMobileStation;

                // voice frame
                sprintf(m_state.ftype, "+DMRv        ");

                if (m_opts.errorbars == 1)
                {
                    printFrameSync(" +DMRv     ", m_synctest_pos + 1);
                }

                m_lastSyncType = DSDSyncDMRVoiceMS;
                m_mbeRate = DSDMBERate3600x2450;
                return (int) DSDSyncDMRVoiceMS; // done
            }
        }
        if (m_opts.frame_provoice == 1)
        {
            if ((memcmp(m_dsdSymbol.getSyncDibitBack(32), m_syncProVoice, 32) == 0)
             || (memcmp(m_dsdSymbol.getSyncDibitBack(32), m_syncProVoiceEA, 32) == 0))
            {
                m_state.carrier = 1;
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
            else if ((memcmp(m_dsdSymbol.getSyncDibitBack(32), m_syncProVoiceInv, 32) == 0)
                || (memcmp(m_dsdSymbol.getSyncDibitBack(32), m_syncProVoiceEAInv, 32) == 0))
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
            if (memcmp(m_dsdSymbol.getSyncDibitBack(19), m_syncNXDNRDCHFull, 19) == 0) // long sync (with preamble)
            {
                m_nxdnInterSyncCount = 0;
				m_state.carrier = 1;
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
            else if (memcmp(m_dsdSymbol.getSyncDibitBack(19), m_syncNXDNRDCHFullInv, 19) == 0) // long sync (with preamble) inverted
            {
                m_nxdnInterSyncCount = 0;
				m_state.carrier = 1;
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
            else if (memcmp(m_dsdSymbol.getSyncDibitBack(10), m_syncNXDNRDCHFSW, 10) == 0) // short sync
            {
                if ((m_nxdnInterSyncCount > 0) && (m_nxdnInterSyncCount % 192 == 0))
                {
                    m_state.carrier = 1;
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
                    m_nxdnInterSyncCount = 0;
                    return (int) DSDSyncNXDNP; // done
                } else {
                    m_nxdnInterSyncCount = 0;
                }
            }
            else if (memcmp(m_dsdSymbol.getSyncDibitBack(10), m_syncNXDNRDCHFSWInv, 10) == 0) // short sync inverted
            {
                if ((m_nxdnInterSyncCount > 0) && (m_nxdnInterSyncCount % 192 == 0))
                {
                    m_state.carrier = 1;
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
                    m_nxdnInterSyncCount = 0;
                    return (int) DSDSyncNXDNN; // done
                } else {
                    m_nxdnInterSyncCount = 0;
                }
            }
        }
        if (m_opts.frame_dpmr == 1)
        {
            if(memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDPMRFS1, 24) == 0) // dPMR classic (not packet)
            {
                m_state.carrier = 1;
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
        }
        if (m_opts.frame_dstar == 1)
        {
            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDStar, 24) == 0)
            {
                m_state.carrier = 1;
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
            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDStarInv, 24) == 0)
            {
                m_state.carrier = 1;
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
            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDStarHeader, 24) == 0)
            {
                m_state.carrier = 1;
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
            if (memcmp(m_dsdSymbol.getSyncDibitBack(24), m_syncDStarHeaderInv, 24) == 0)
            {
                m_state.carrier = 1;
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

    if (m_nxdnInterSyncCount >= 0) { // increment only if a first match was encoutnered (therefore set to 0)
        m_nxdnInterSyncCount++;
    }

    m_synctest_pos++;

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

    return -2; // still searching
}

void DSDDecoder::resetFrameSync()
{
    m_dsdLogger.log("DSDDecoder::resetFrameSync: symbol %d (%d)\n", m_state.symbolcnt, m_dsdSymbol.getSymbol());

    // reset detect frame sync engine
    m_t = 0;
    m_synctest_pos = 0;

    m_sync = -2;   // mark in progress

    if ((m_opts.symboltiming == 1) && (m_state.carrier == 1))
    {
        m_dsdLogger.log("\nSymbol Timing:\n");
    }


    m_nxdnInterSyncCount = -1;   // reset to quiet state
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
    m_dsdSymbol.noCarrier();

    m_stationType = DSDStationTypeNotApplicable;
    m_lastSyncType = DSDSyncNone;
    m_state.carrier = 0;

    sprintf(m_state.slot0light, "                          ");
    sprintf(m_state.slot1light, "                          ");
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

    m_state.firstframe = 0;

    sprintf(m_state.algid, "________");
    sprintf(m_state.keyid, "________________");
    m_mbeDecoder1.initMbeParms();
    m_mbeDecoder2.initMbeParms();

    m_voice1On = false;
    m_voice2On = false;
}

void DSDDecoder::setTDMAStereo(bool tdmaStereo)
{
    if (tdmaStereo)
    {
        m_mbeDecoder1.setChannels(1);
        m_mbeDecoder2.setChannels(2);
    }
    else
    {
        m_mbeDecoder1.setChannels(3);
        m_mbeDecoder2.setChannels(3);
    }
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

void DSDDecoder::formatStatusText(char *statusText)
{
    uint64_t tv_sec, tv_msec;
    uint64_t nowms = TimeUtil::nowms();
    tv_sec = nowms / 1000;
    tv_msec = nowms % 1000;
    sprintf(statusText, "%d.%03d:", (uint32_t) tv_sec, (uint32_t) tv_msec);

    switch (getSyncType())
    {
    case DSDcc::DSDDecoder::DSDSyncDMRDataMS:
    case DSDcc::DSDDecoder::DSDSyncDMRDataP:
    case DSDcc::DSDDecoder::DSDSyncDMRVoiceMS:
    case DSDcc::DSDDecoder::DSDSyncDMRVoiceP:
        if (m_signalFormat != signalFormatDMR)
        {
            strcpy(&statusText[15], "DMR>Sta: __ S1: __________________________ S2: __________________________");
        }
        else
        {
        	memcpy(&statusText[15], "DMR", 3);
        }

        switch (getStationType())
        {
        case DSDcc::DSDDecoder::DSDBaseStation:
            memcpy(&statusText[24], "BS ", 3);
            break;
        case DSDcc::DSDDecoder::DSDMobileStation:
            memcpy(&statusText[24], "MS ", 3);
            break;
        default:
            memcpy(&statusText[24], "NA ", 3);
            break;
        }

        memcpy(&statusText[31], getDMRDecoder().getSlot0Text(), 26);
        memcpy(&statusText[62], getDMRDecoder().getSlot1Text(), 26);
        m_signalFormat = signalFormatDMR;
        break;
    case DSDcc::DSDDecoder::DSDSyncDStarHeaderN:
    case DSDcc::DSDDecoder::DSDSyncDStarHeaderP:
    case DSDcc::DSDDecoder::DSDSyncDStarN:
    case DSDcc::DSDDecoder::DSDSyncDStarP:
        if (m_signalFormat != signalFormatDStar)
        {
                                  // 1    2    2    3    3    4    4    5    5    6    6    7    7    8    8    9    9
                                  // 5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5...
            strcpy(&statusText[15], "DST>________/____>________|________>________|____________________|______:___/_____._");
                                      // MY            UR       RPT1     RPT2     Info                 Loc    Target
        }
        else
        {
        	memcpy(&statusText[15], "DST", 3);
        }

        {
            const std::string& rpt1 = getDStarDecoder().getRpt1();
            const std::string& rpt2 = getDStarDecoder().getRpt2();
            const std::string& mySign = getDStarDecoder().getMySign();
            const std::string& yrSign = getDStarDecoder().getYourSign();

            if (rpt1.length() > 0) { // 0 or 8
                memcpy(&statusText[42], rpt1.c_str(), 8);
            }
            if (rpt2.length() > 0) { // 0 or 8
                memcpy(&statusText[51], rpt2.c_str(), 8);
            }
            if (yrSign.length() > 0) { // 0 or 8
                memcpy(&statusText[33], yrSign.c_str(), 8);
            }
            if (mySign.length() > 0) { // 0 or 13
                memcpy(&statusText[19], mySign.c_str(), 13);
            }
            memcpy(&statusText[60], getDStarDecoder().getInfoText(), 20);
            memcpy(&statusText[81], getDStarDecoder().getLocator(), 6);
            sprintf(&statusText[88], "%03d/%07.1f",
                    getDStarDecoder().getBearing(),
                    getDStarDecoder().getDistance());
        }

        statusText[101] = '\0';
        m_signalFormat = signalFormatDStar;
        break;
    case DSDcc::DSDDecoder::DSDSyncDPMR:
        sprintf(&statusText[15], "DPM>%s CC: %04d OI: %08d CI: %08d",
                DSDcc::DSDdPMR::dpmrFrameTypes[(int) getDPMRDecoder().getFrameType()],
                getDPMRDecoder().getColorCode(),
                getDPMRDecoder().getOwnId(),
                getDPMRDecoder().getCalledId());
        m_signalFormat = signalFormatDPMR;
        break;
    case DSDcc::DSDDecoder::DSDSyncYSF:
        //           1    1    2    2    3    3    4    4    5    5    6    6    7    7    8
        // 0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0..
        // C V2 RI 0:7 WL000|ssssssssss>dddddddddd |UUUUUUUUUU>DDDDDDDDDD|44444
        if (getYSFDecoder().getFICHError() == DSDcc::DSDYSF::FICHNoError)
        {
            sprintf(&statusText[15], "YSF>%s ", DSDcc::DSDYSF::ysfChannelTypeText[(int) getYSFDecoder().getFICH().getFrameInformation()]);
        }
        else
        {
            sprintf(&statusText[15], "YSF>%d ", (int) getYSFDecoder().getFICHError());
        }

        sprintf(&statusText[21], "%s %s %d:%d %c%c",
                DSDcc::DSDYSF::ysfDataTypeText[(int) getYSFDecoder().getFICH().getDataType()],
                DSDcc::DSDYSF::ysfCallModeText[(int) getYSFDecoder().getFICH().getCallMode()],
                getYSFDecoder().getFICH().getBlockTotal(),
                getYSFDecoder().getFICH().getFrameTotal(),
                (getYSFDecoder().getFICH().isNarrowMode() ? 'N' : 'W'),
                (getYSFDecoder().getFICH().isInternetPath() ? 'I' : 'L'));

        if (getYSFDecoder().getFICH().isSquelchCodeEnabled())
        {
            sprintf(&statusText[33], "%03d", getYSFDecoder().getFICH().getSquelchCode());
        }
        else
        {
            strcpy(&statusText[33], "---");
        }

        char dest[11+1];

        if ( getYSFDecoder().radioIdMode())
        {
            sprintf(dest, "%-5s:%-5s",
                    getYSFDecoder().getDestId(),
                    getYSFDecoder().getSrcId());
        }
        else
        {
            sprintf(dest, "%-10s", getYSFDecoder().getDest());
        }

        sprintf(&statusText[36], "|%-10s>%s|%-10s>%-10s|%-5s",
                getYSFDecoder().getSrc(),
                dest,
                getYSFDecoder().getUplink(),
                getYSFDecoder().getDownlink(),
                getYSFDecoder().getRem4());

        m_signalFormat = signalFormatYSF;
        break;
    case DSDcc::DSDDecoder::DSDSyncNXDNN:
    case DSDcc::DSDDecoder::DSDSyncNXDNP:
        if (getNXDNDecoder().getRFChannel() == DSDNXDN::NXDNRCCH)
        {
            // 1    2    2    3    3    4    4    5    5    6    6    7    7    8
            // 5....0....5....0....5....0....5....0....5....0....5....0....5....0..
            // NXD>RC r cc mm llllll ssss
            sprintf(&statusText[15], "NXD>RC %s %02d %02X %06X %02X",
                getNXDNDecoder().isFullRate() ? "F" : "H",
                getNXDNDecoder().getRAN(),
                getNXDNDecoder().getMessageType(),
                getNXDNDecoder().getLocationId(),
                getNXDNDecoder().getServicesFlag());
        }
        else if ((getNXDNDecoder().getRFChannel() == DSDNXDN::NXDNRTCH)
            || (getNXDNDecoder().getRFChannel() == DSDNXDN::NXDNRDCH))
        {
            if (getNXDNDecoder().isIdle()) {
                snprintf(&statusText[15], 82, "NXD>%s IDLE", getNXDNDecoder().getRFChannelStr());
            }
            else
            {
                // 1    2    2    3    3    4    4    5    5    6    6    7    7    8
                // 5....0....5....0....5....0....5....0....5....0....5....0....5....0..
                // NXD>Rx r cc mm sssss>gddddd
                snprintf(&statusText[15], 82, "NXD>%s %s %02d %02X %05d>%c%05d",
                        getNXDNDecoder().isFullRate() ? "F" : "H",
                        getNXDNDecoder().getRFChannelStr(),
                        getNXDNDecoder().getRAN(),
                        getNXDNDecoder().getMessageType(),
                        getNXDNDecoder().getSourceId(),
                        getNXDNDecoder().isGroupCall() ? 'G' : 'I',
                        getNXDNDecoder().getDestinationId());
            }
        }
        else
        {
            // 1    2    2    3    3    4    4    5    5    6    6    7    7    8
            // 5....0....5....0....5....0....5....0....5....0....5....0....5....0..
            // NXD>RU
            snprintf(&statusText[15], 82, "NXD>RU");
        }
        m_signalFormat = signalFormatNXDN;
        break;
    default:
    	strcpy(&statusText[15], "XXX>");
        m_signalFormat = signalFormatNone;
        break;
    }

    statusText[101] = '\0'; // guard
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
