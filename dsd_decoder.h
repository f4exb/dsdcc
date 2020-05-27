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

#ifndef DSDCC_DSD_DECODER_H_
#define DSDCC_DSD_DECODER_H_

#include "dsd_opts.h"
#include "dsd_state.h"
#include "dsd_logger.h"
#include "dsd_symbol.h"
#include "dsd_mbe.h"
#include "dmr.h"
#include "ysf.h"
#include "dpmr.h"
#include "dstar.h"
#include "nxdn.h"
#include "locator.h"
#include "export.h"

#define DSD_SQUELCH_TIMEOUT_SAMPLES 960 // 200ms timeout after return to sync search

namespace DSDcc
{

class DSDCC_API DSDDecoder
{
    friend class DSDSymbol;
    friend class DSDMBEDecoder;
    friend class DSDDMR;
    friend class DSDDstar;
    friend class DSDDstarOld;
    friend class DSDYSF;
    friend class DSDdPMR;
    friend class DSDNXDN;
public:
    typedef enum
    {
        DSDDecodeAuto,
        DSDDecodeNone,
        DSDDecodeP25P1,
        DSDDecodeDStar,
        DSDDecodeNXDN48,
        DSDDecodeNXDN96,
        DSDDecodeProVoice,
        DSDDecodeDMR,
        DSDDecodeX2TDMA,
        DSDDecodeDPMR,
        DSDDecodeYSF
    } DSDDecodeMode;

    typedef enum
    {
        DSDRate2400,
        DSDRate4800,
        DSDRate9600
    } DSDRate;

    typedef enum
    {
        DSDShowP25EncryptionSyncBits,
        DSDShowP25LinkControlBits,
        DSDShowP25StatusBitsAndLowSpeedData,
        DSDShowP25TalkGroupInfo
    } DSDShowP25;

    typedef enum
    {
        DSDLookForSync,
        DSDSyncFound,
        DSDprocessFrame,
        DSDprocessNXDNVoice,
        DSDprocessNXDNData,
        DSDprocessDSTAR,
        DSDprocessDSTAR_HD,
        DSDprocessDMRvoice,
        DSDprocessDMRdata,
        DSDprocessDMRvoiceMS,
        DSDprocessDMRdataMS,
		DSDprocessDMRsyncOrSkip,
        DSDprocessDMRSkipMS,
        DSDprocessX2TDMAvoice,
        DSDprocessX2TDMAdata,
        DSDprocessProVoice,
        DSDprocessYSF,
        DSDprocessDPMR,
		DSDprocessNXDN,
        DSDprocessUnknown
    } DSDFSMState;

    typedef enum
    {
        DSDSyncP25p1P,         // 0
        DSDSyncP25p1N,         // 1
        DSDSyncX2TDMADataP,    // 2
        DSDSyncX2TDMAVoiceN,   // 3
        DSDSyncX2TDMAVoiceP,   // 4
        DSDSyncX2TDMADataN,    // 5
        DSDSyncDStarP,         // 6
        DSDSyncDStarN,         // 7
        DSDSyncNXDNP,          // 8
        DSDSyncNXDNN,          // 9
        DSDSyncDMRDataP,       // 10
        DSDSyncDMRDataMS,      // 11
        DSDSyncDMRVoiceP,      // 12
        DSDSyncDMRVoiceMS,     // 13
        DSDSyncProVoiceP,      // 14
        DSDSyncProVoiceN,      // 15
        DSDSyncNXDNDataP,      // 16
        DSDSyncNXDNDataN,      // 17
        DSDSyncDStarHeaderP,   // 18
        DSDSyncDStarHeaderN,   // 19
        DSDSyncDPMR,           // 20
        DSDSyncDPMRPacket,     // 21
        DSDSyncDPMRPayload,    // 22
        DSDSyncDPMREnd,        // 23
        DSDSyncYSF,            // 24
        DSDSyncNone
    } DSDSyncType;

    typedef enum
    {
        DSDStationTypeNotApplicable,
        DSDBaseStation,
        DSDMobileStation
    } DSDStationType;

    typedef enum
    {
        DSDMBERateNone,
        DSDMBERate3600x2400, //!< D-Star
        DSDMBERate3600x2450, //!< DMR, dPMR, YSF V/D type 1, NXDN
        DSDMBERate7200x4400,
        DSDMBERate7100x4400,
        DSDMBERate2400,
        DSDMBERate2450,      //!< YSF V/D type 2 (does not use FEC in AMBE codec)
        DSDMBERate4400
    } DSDMBERate;

    DSDDecoder();
    ~DSDDecoder();

    void run(short sample);
    short getFilteredSample() const { return m_dsdSymbol.getFilteredSample(); }
    short getSymbolSyncSample() const { return m_dsdSymbol.getSymbolSyncSample(); }

    /** DVSI support */

    const unsigned char *getMbeDVFrame1() const {
        return m_mbeDVFrame1;
    }

    bool mbeDVReady1() const {
        return m_mbeDVReady1;
    }

    void resetMbeDV1() {
        m_mbeDVReady1 = false;
    }

    const unsigned char *getMbeDVFrame2() const {
        return m_mbeDVFrame2;
    }

    bool mbeDVReady2() const {
        return m_mbeDVReady2;
    }

    void resetMbeDV2() {
        m_mbeDVReady2 = false;
    }

    /** MBElib support */

    short *getAudio1(int& nbSamples)
    {
        return m_mbeDecoder1.getAudio(nbSamples);
    }

    void resetAudio1()
    {
        m_mbeDecoder1.resetAudio();
    }

    short *getAudio2(int& nbSamples)
    {
        return m_mbeDecoder2.getAudio(nbSamples);
    }

    void resetAudio2()
    {
        m_mbeDecoder2.resetAudio();
    }

    //DSDOpts *getOpts() { return &m_opts; }
    //DSDState *getState() { return &m_state; }

    void setLogVerbosity(int verbosity) { m_dsdLogger.setVerbosity(verbosity); }
    void setLogFile(const char *filename) { m_dsdLogger.setFile(filename); }
    const DSDLogger& getLogger() const { return m_dsdLogger; }

    DSDSyncType getSyncType() const
    {
        return m_lastSyncType;
    }

    DSDStationType getStationType() const { return m_stationType; }
    const char *getFrameTypeText() const { return m_state.ftype; }
    const char *getFrameSubtypeText() const { return m_state.fsubtype; }
    int getInLevel() const { return m_dsdSymbol.getLevel(); }
    int getCarrierPos() const { return m_dsdSymbol.getCarrierPos(); }
    int getZeroCrossingPos() const { return m_dsdSymbol.getZeroCrossingPos(); }
    int getSymbolSyncQuality() const { return m_dsdSymbol.getSymbolSyncQuality(); }
    int getSamplesPerSymbol() const { return m_dsdSymbol.getSamplesPerSymbol(); }
    DSDRate getDataRate() const { return m_dataRate; };
    bool getVoice1On() const { return m_voice1On; }
    bool getVoice2On() const { return m_voice2On; }
    void setTDMAStereo(bool tdmaStereo);
    void formatStatusText(char *statusText);
    bool getSymbolPLLLocked() const { return m_dsdSymbol.getPLLLocked(); }

    const DSDDMR& getDMRDecoder() const { return m_dsdDMR; }
    const DSDDstar& getDStarDecoder() const { return m_dsdDstar; }
    const DSDdPMR& getDPMRDecoder() const { return m_dsdDPMR; }
    const DSDYSF& getYSFDecoder() const { return m_dsdYSF; }
    const DSDNXDN& getNXDNDecoder() const { return m_dsdNXDN; }
    void enableMbelib(bool enable) { m_mbelibEnable = enable; }

    // Initializations:
    void setQuiet();
    void setVerbosity(int verbosity);
    void showErrorBars();
    void showSymbolTiming();
    void setP25DisplayOptions(DSDShowP25 mode, bool on);
    void muteEncryptedP25(bool on);
    void setDecodeMode(DSDDecodeMode mode, bool on);
    void setAudioGain(float gain);
    void setUvQuality(int uvquality);
    void setUpsampling(int upsampling);
    void setStereo(bool on);
    void setInvertedXTDMA(bool on);
    void enableCosineFiltering(bool on);
    void enableAudioOut(bool on);
    void enableScanResumeAfterTDULCFrames(int nbFrames);
    void setDataRate(DSDRate dataRate);
    void setMyPoint(float lat, float lon) { m_myPoint.setLatLon(lat, lon); }
    void setSymbolPLLLock(bool pllLock) { m_dsdSymbol.setPLLLock(pllLock); }
    void setDMRBasicPrivacyKey(unsigned char key);

    // parameter getters:

    int upsampling() const { return m_mbeDecoder1.getUpsamplingFactor(); }

    DSDMBERate getMbeRate() const { return m_mbeRate; }
    void setMbeRate(DSDMBERate mbeRate) { m_mbeRate = mbeRate; }

    void useHPMbelib(bool useHP) {
        m_mbeDecoder1.useHP(useHP);
        m_mbeDecoder2.useHP(useHP);
    }

    /*
     * Frame sync patterns
     */
    static const unsigned char m_syncDMRDataBS[24];
    static const unsigned char m_syncDMRVoiceBS[24];
    static const unsigned char m_syncDMRDataMS[24];
    static const unsigned char m_syncDMRVoiceMS[24];
    static const unsigned char m_syncDPMRFS1[24];
    static const unsigned char m_syncDPMRFS4[24];
    static const unsigned char m_syncDPMRFS2[12];
    static const unsigned char m_syncDPMRFS3[12];
    static const unsigned char m_syncNXDNRDCHFull[19];
    static const unsigned char m_syncNXDNRDCHFullInv[19];
    static const unsigned char m_syncNXDNRDCHFSW[10];
    static const unsigned char m_syncNXDNRDCHFSWInv[10];
    static const unsigned char m_syncDStarHeader[24];
    static const unsigned char m_syncDStarHeaderInv[24];
    static const unsigned char m_syncDStar[24];
    static const unsigned char m_syncDStarInv[24];
    static const unsigned char m_syncYSF[20];
    static const unsigned char m_syncP25P1[24];
    static const unsigned char m_syncP25P1Inv[24];
    static const unsigned char m_syncX2TDMADataBS[24];
    static const unsigned char m_syncX2TDMAVoiceBS[24];
    static const unsigned char m_syncX2TDMADataMS[24];
    static const unsigned char m_syncX2TDMAVoiceMS[24];
    static const unsigned char m_syncProVoice[32];
    static const unsigned char m_syncProVoiceInv[32];
    static const unsigned char m_syncProVoiceEA[32];
    static const unsigned char m_syncProVoiceEAInv[32];

private:
    typedef enum
    {
        signalFormatNone,
        signalFormatDMR,
        signalFormatDStar,
        signalFormatDPMR,
        signalFormatYSF,
        signalFormatNXDN
    } SignalFormat;

    int getFrameSync();
    void resetFrameSync();
    void printFrameSync(const char *frametype, int offset);
    void noCarrier();
    void printFrameInfo();
    void processFrameInit();
    static int comp(const void *a, const void *b);

    DSDOpts m_opts;
    DSDState m_state;
    DSDLogger m_dsdLogger;
    DSDFSMState m_fsmState;
    DSDStationType m_stationType;
    DSDDMR::DSDDMRBurstType m_dmrBurstType;
    // sync engine:
    int m_sync; //!< The current internal sync type
    int m_dibit, m_synctest_pos;
    int m_lsum;
    char m_spectrum[64];
    int m_t;
    int m_squelchTimeoutCount;
    int m_nxdnInterSyncCount;
    // Symbol extraction and operations
    DSDSymbol m_dsdSymbol;
    // MBE decoder
    char ambe_fr[4][24];
    char imbe_fr[8][23];
    bool m_mbelibEnable;
    DSDMBERate m_mbeRate;
    DSDMBEDecoder m_mbeDecoder1; //!< AMBE decoder for TDMA unique or first slot
    DSDMBEDecoder m_mbeDecoder2; //!< AMBE decoder for TDMA second slot
    // DVSI AMBE3000 serial device support
    unsigned char m_mbeDVFrame1[18]; //!< AMBE/IMBE encoded frame for TDMA unique or first slot
    bool m_mbeDVReady1;              //!< AMBE/IMBE encoded frame ready status for TDMA unique or first slot
    unsigned char m_mbeDVFrame2[9];  //!< AMBE encoded frame for TDMA second slot
    bool m_mbeDVReady2;              //!< AMBE encoded frame ready status for TDMA second slot
    // Voice announcements
    bool m_voice1On;
    bool m_voice2On;
    // Frame decoders
    DSDDMR m_dsdDMR;
    DSDDstar m_dsdDstar;
    DSDYSF m_dsdYSF;
    DSDdPMR m_dsdDPMR;
    DSDNXDN m_dsdNXDN;
    DSDRate m_dataRate;
    DSDSyncType m_syncType;
    DSDSyncType m_lastSyncType;
    LocPoint m_myPoint;
    // status text
    SignalFormat m_signalFormat;
};

} // namespace dsdcc

#endif /* DSDCC_DSD_DECODER_H_ */
