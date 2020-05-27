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

#ifndef DMR_H_
#define DMR_H_

#include "fec.h"
#include "export.h"

#define DMR_TYPES_COUNT 12
#define DMR_SLOT_TYPE_PARITY_LEN 12

#define DMR_TS_LEN 288              //!< Length of 30ms frame in bits
#define DMR_CACH_LEN 24
#define DMR_SYNC_LEN 48

#define DMR_EMB_PART_LEN 8
#define DMR_ES_LEN 32               //!< Embedded signaling length
#define DMR_SLOT_TYPE_PART_LEN 10

#define DMR_VOCODER_FRAME_LEN 72    //!< Size in bits of Vocoder frame (20ms)
#define DMR_VOX_PART_LEN 108
#define DMR_DATA_PART_LEN 98

#define DMR_VOX_SUPERFRAME_LEN 6    //!< Count of frames in Superframe
#define IN_DIBITS(x) ((x)/2)        //!< Convert size to dibit units count
#define IN_BYTES(x) ((x)/8)
#define DMR_BP_KEYS_COUNT 255   //!< Basic Privacy Keys count

typedef enum
{
    SingleLC_FirstCSBK,      // 0
    FirstLC,                 // 1
    LastLC_CSBK,             // 2
    ContLC_CSBK              // 3
} DMRLcss;

namespace DSDcc
{

class DSDDecoder;

class DSDCC_API DSDDMR
{
public:
    typedef enum
    {
        DSDDMRBurstNone,
        DSDDMRBaseStation,       //!< BS sourced voice or data
        DSDDMRMobileStation,     //!< MS sourced voice or data
        DSDDMRMobileStationRC,   //!< MS sourced standalone RC
        DSDDMRDirectSlot1,       //!< TDMA direct mode time slot 1 voice or data
        DSDDMRDirectSlot2        //!< TDMA direct mode time slot 2 voice or data
    } DSDDMRBurstType;

    typedef enum
    {
        DSDDMRSlot1,             // 0
        DSDDMRSlot2,             // 1
        DSDDMRSlotUndefined      // 2
    } DSDDMRSlot;

    typedef enum
    {
        DSDDMRDataPIHeader,         // 0
        DSDDMRDataVoiceLCHeader,    // 1
        DSDDMRDataTerminatorWithLC, // 2
        DSDDMRDataCSBK,             // 3
        DSDDMRDataMBCHeader,        // 4
        DSDDMRDataMBCContinuation,  // 5
        DSDDMRDataDataHeader,       // 6
        DSDDMRDataRate_1_2_Data,    // 7
        DSDDMRDataRate_3_4_Data,    // 8
        DSDDMRDataIdle,             // 9
        DSDDMRDataRate_1,           // 10
        DSDDMRDataUnifiedSingleBlock, // 11
        DSDDMRDataReserved,
        DSDDMRDataUnknown
    } DSDDMRDataTYpe;

    explicit DSDDMR(DSDDecoder *dsdDecoder);
    ~DSDDMR();

    void initData();
    void initVoice();
    void processData();
    void processVoice();
    void processSyncOrSkip();

    void initVoiceMS();
    void processVoiceMS();
    void processSkipMS();
    void initDataMS();
    void processDataMS();

    const char *getSlot0Text() const;
    const char *getSlot1Text() const;
    unsigned char getColorCode() const;

private:
    struct DMRAddresses
    {
        DMRAddresses() :
            m_group(false),
            m_target(0),
            m_source(0)
        {
        }

        bool         m_group;
        unsigned int m_target;
        unsigned int m_source;
    };

    void processDataFirstHalf(unsigned int shiftBack);  //!< Because sync is in the middle of a frame you need to process the first half first: CACH to end of SYNC
    void processVoiceFirstHalf(unsigned int shiftBack); //!< Because sync is in the middle of a frame you need to process the first half first: CACH to end of SYNC
    void decodeCACH(unsigned char *cachBits);
    void processSlotTypePDU();
    bool processEMB();
    bool processVoiceEmbeddedSignalling(int& voiceEmbSig_dibitsIndex, unsigned char *voiceEmbSigRawBits, bool& voiceEmbSig_OK, DMRAddresses& addresses);
    void processVoiceDibit(unsigned char dibit);
    void processDataDibit(unsigned char dibit);
    void storeSymbolDV(unsigned char *mbeFrame, int dibitindex, unsigned char dibit, bool invertDibit = false);
    static void textVoiceEmbeddedSignalling(DMRAddresses& addresses, char *slotText);

    void processVoiceFirstHalfMS();
    void processDataFirstHalfMS();

    void BasicPrivacyXOR(unsigned char *dibit, int pos);

    DSDDecoder *m_dsdDecoder;
    int  m_symbolIndex;                   //!< current symbol index in non HD sequence
    int  m_cachSymbolIndex;               //!< count of symbols since last positive CACH identification
    DSDDMRBurstType m_burstType;
    DSDDMRSlot m_slot;
    bool m_continuation;
    bool m_cachOK;
    unsigned char m_lcss;
    unsigned char m_colorCode;
    DSDDMRDataTYpe m_dataType;
    char *m_slotText;
    unsigned char m_slotTypePDU_dibits[10];
    unsigned char m_cachBits[24];
    unsigned char m_emb_dibits[8];
    unsigned char m_voiceEmbSig_dibits[16];
    unsigned char m_voice1EmbSigRawBits[16*8];
    int           m_voice1EmbSig_dibitsIndex;
    bool          m_voice1EmbSig_OK;
    DMRAddresses  m_slot1Addresses;
    unsigned char m_voice2EmbSigRawBits[16*8];
    int           m_voice2EmbSig_dibitsIndex;
    bool          m_voice2EmbSig_OK;
    DMRAddresses  m_slot2Addresses;
    unsigned char m_syncDibits[24];
    unsigned int m_voice1FrameCount; //!< current frame count in voice superframe: [0..5] else no superframe on going
    unsigned int m_voice2FrameCount; //!< current frame count in voice superframe: [0..5] else no superframe on going
    unsigned char m_mbeDVFrame[9];

    Hamming_7_4 m_hamming_7_4;
    Golay_20_8 m_golay_20_8;
    QR_16_7_6 m_qr_16_7_6;
    Hamming_16_11_4 m_hamming_16_11_4;

    const int *w, *x, *y, *z;

    static const int m_cachInterleave[24];
    static const int m_embSigInterleave[128];
    static const char *m_slotTypeText[DMR_TYPES_COUNT];

    static const int rW[36];
    static const int rX[36];
    static const int rY[36];
    static const int rZ[36];
    static const unsigned short BasicPrivacyKeys[DMR_BP_KEYS_COUNT];
};

} // namespace DSDcc



#endif /* DMR_H_ */
