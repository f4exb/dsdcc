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
#include "dmr_voice.h"
#include "dmr_data.h"
#include "dmr.h"
#include "dstar.h"
#include "ysf.h"
#include "dpmr.h"
#include "nxdn.h"

#define DSD_SQUELCH_TIMEOUT_SAMPLES 960 // 200ms timeout after return to sync search

/*
 * Frame sync patterns
 */
#define INV_P25P1_SYNC "333331331133111131311111"
#define P25P1_SYNC     "111113113311333313133333"

#define X2TDMA_BS_VOICE_SYNC "113131333331313331113311"
#define X2TDMA_BS_DATA_SYNC  "331313111113131113331133"
#define X2TDMA_MS_DATA_SYNC  "313113333111111133333313"
#define X2TDMA_MS_VOICE_SYNC "131331111333333311111131"

#define DSTAR_HD       "131313131333133113131111"
#define INV_DSTAR_HD   "313131313111311331313333"
#define DSTAR_SYNC     "313131313133131113313111"
#define INV_DSTAR_SYNC "131313131311313331131333"

// NXDN symbol mapping: 01(1):+3, 00(0):+1, 10(2):-1, 11(3):-3
// Preamble and FSW only for RDCH (conventional) type
//                           PREAMBLE  FSW
#define NXDN_RDCH_FULL_SYNC "11131133313131331131" // Full sync lookup in auto mode
#define NXDN_RDCH_FSW_SYNC            "3131331131" // FSW sync lookup (follow up or sync lookup in NXDN mode)
// inverted versions
#define INV_NXDN_RDCH_FULL_SYNC "33313311131313113313"
#define INV_NXDN_RDCH_FSW_SYNC            "1313113313"

#define DMR_BS_DATA_SYNC  "313333111331131131331131" // DF F5 7D 75 DF 5D
#define DMR_BS_VOICE_SYNC "131111333113313313113313" // 75 5F D7 DF 75 F7
#define DMR_MS_DATA_SYNC  "311131133313133331131113" // D5 D7 F7 7F D7 57
#define DMR_MS_VOICE_SYNC "133313311131311113313331" // 7F 7D 5D D5 7D FD

#define INV_PROVOICE_SYNC    "31313111333133133311331133113311"
#define PROVOICE_SYNC        "13131333111311311133113311331133"
#define INV_PROVOICE_EA_SYNC "13313133113113333311313133133311"
#define PROVOICE_EA_SYNC     "31131311331331111133131311311133"

// dPMR symbol mapping: 01(1):+3, 00(0):+1, 10(2):-1, 11(3):-3
#define DPMR_FS1_SYNC "111333331133131131111313" // 57 FF 5F 75 D5 77 - non packet data header
#define DPMR_FS2_SYNC "113333131331" // 5F F7 7D                      - superframe sync (each 2 384 bit frames)
#define DPMR_FS3_SYNC "133131333311" // 7D DF F5                      - end frame sync
#define DPMR_PREAMBLE "113311331133" // 5F 5F 5F ...                  - preamble sequence
#define DPMR_FS4_SYNC "333111113311313313333131"// FD 55 F5 DF 7F DD  - packet data header

#define YSF_SYNC "31111311313113131131" // D4 71 C9 63 4D => D5 75 DD 77 5D

namespace DSDcc
{

class DSDDecoder
{
    friend class DSDSymbol;
    friend class DSDMBEDecoder;
    friend class DSDDMRVoice;
    friend class DSDDMRData;
    friend class DSDDMR;
    friend class DSDDstar;
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
        DSDprocessDMRvoiceBS,
        DSDprocessDMRdataBS,
        DSDprocessDMRvoice,
        DSDprocessDMRdata,
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
        DSDSyncDMRVoiceN,      // 11
        DSDSyncDMRVoiceP,      // 12
        DSDSyncDMRDataN,       // 13
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
        DSDMBERate3600x2450, //!< DMR, dPMR and the likes
        DSDMBERate7200x4400,
        DSDMBERate7100x4400
    } DSDMBERate;

    DSDDecoder();
    ~DSDDecoder();

    void run(short sample);
    short getFilteredSample() const { return m_dsdSymbol.getFilteredSample(); }
    short getSymbolSyncSample() const { return m_dsdSymbol.getSymbolSyncSample(); }

    /** DVSI support */
    const unsigned char *getMbeDVFrame() const {
        return m_mbeDVFrame;
    }

    DSDMBERate getMbeRate() const {
        return m_mbeRate;
    }

    bool mbeDVReady() const {
        return m_mbeDVReady;
    }

    void resetMbeDV() {
        m_mbeDVReady = false;
    }

    short *getAudio1(int& nbSamples)
    {
        return m_mbeDecoder1.getAudio(nbSamples);
    }

    void resetAudio1()
    {
        m_mbeDecoder1.resetAudio();
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

    const DSDStationType getStationType() const { return m_stationType; }
    const char *getFrameTypeText() const { return m_state.ftype; }
    const char *getFrameSubtypeText() const { return m_state.fsubtype; }
//    const char *getModulationText() const { return m_modulation; }
    const char *getSlot0Text() const { return m_state.slot0light; }
    const char *getSlot1Text() const { return m_state.slot1light; }
    unsigned char getColorCode() const { return m_state.ccnum; }
    int getInLevel() const { return m_dsdSymbol.getLevel(); }
    int getCarrierPos() const { return m_dsdSymbol.getCarrierPos(); }
    int getZeroCrossingPos() const { return m_dsdSymbol.getZeroCrossingPos(); }
    int getSymbolSyncQuality() const { return m_dsdSymbol.getSymbolSyncQuality(); }
    int getSamplesPerSymbol() const { return m_dsdSymbol.getSamplesPerSymbol(); }
    DSDRate getDataRate() const { return m_dataRate; };
    const DSDDstar& getDStarDecoder() const { return m_dsdDstar; }
    const DSDdPMR& getDPMRDecoder() const { return m_dsdDPMR; }
    void enableMbelib(bool enable) { m_mbelibEnable = enable; }

    // Initializations:
    void setQuiet();
    void setVerbosity(int verbosity);
    void showDatascope();
    void setDatascopeFrameRate(int frameRate);
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
    void setAutoDetectionThreshold(int threshold);
    void setQPSKSymbolBufferSize(int size);
    void setQPSKMinMaxBufferSize(int size);
    void enableCosineFiltering(bool on);
    void enableAudioOut(bool on);
    void enableScanResumeAfterTDULCFrames(int nbFrames);
    void setDataRate(DSDRate dataRate);

    // parameter getters:
    int upsampling() const { return m_mbeDecoder1.getUpsamplingFactor(); }

private:
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
    char m_synctest[25];   //!< for default 24 dibits sync word
    char m_synctest18[19]; //!< for 18 dibits sync word
    char m_synctest32[33]; //!< for 32 dibits sync word
    char m_synctest12[13]; //!< for 12 dibits sync word
    char m_synctest20[21]; //!< for 20 dibits sync word
//    char m_modulation[8];
    char *m_synctest_p;
    char m_synctest_buf[10240];
    int m_lsum;
    char m_spectrum[64];
    int m_t;
    int m_squelchTimeoutCount;
    // Symbol extraction and operations
    DSDSymbol m_dsdSymbol;
    // MBE decoder
    char ambe_fr[4][24];
    bool m_mbelibEnable;
    DSDMBERate m_mbeRate;
    DSDMBEDecoder m_mbeDecoder1; //!< AMBE decoder for TDMA unique or first slot
    // DVSI AMBE3000 serial device support
    unsigned char m_mbeDVFrame[9];
    bool m_mbeDVReady;
    // Frame decoders
    DSDDMRVoice m_dsdDMRVoice;
    DSDDMRData m_dsdDMRData;
    DSDDMR m_dsdDMR;
    DSDDstar m_dsdDstar;
    DSDYSF m_dsdYSF;
    DSDdPMR m_dsdDPMR;
    DSDNXDN m_dsdNXDN;
    DSDRate m_dataRate;
    DSDSyncType m_syncType;
    DSDSyncType m_lastSyncType;
};

} // namespace dsdcc

#endif /* DSDCC_DSD_DECODER_H_ */
