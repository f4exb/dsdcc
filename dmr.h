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

namespace DSDcc
{

class DSDDecoder;

class DSDDMR
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
        DSDDMRSlotUndefined,
        DSDDMRSlot1,
        DSDDMRSlot2
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
        DSDDMRDataReserved,
        DSDDMRDataUnknown
    } DSDDMRDataTYpe;

    DSDDMR(DSDDecoder *dsdDecoder);
    ~DSDDMR();

    void initData(DSDDMRBurstType burstType);
    void initVoice(DSDDMRBurstType burstType);
    void processData();
    void processVoice();

private:

    class Hamming_7_4
    {
    public:
        Hamming_7_4();
        ~Hamming_7_4();

        void init();
        bool decode(unsigned char *rxBits);

    private:
        unsigned char m_corr[8];             //!< single bit error correction by syndrome index
        static const unsigned char m_H[7*3]; //!< Parity check matrix of bits
    };

    class Golay_20_8
    {
    public:
        Golay_20_8();
        ~Golay_20_8();

        void init();
        bool decode(unsigned char *rxBits);

    private:
        unsigned char m_corr[4096][3];         //!< up to 3 bit error correction by syndrome index
        static const unsigned char m_H[20*12]; //!< Parity check matrix of bits
    };

    void processDataFirstHalf();  //!< Because sync is in the middle of a frame you need to process the first half first: CACH to end of SYNC
    void processVoiceFirstHalf(); //!< Because sync is in the middle of a frame you need to process the first half first: CACH to end of SYNC
    void processCACH(unsigned char *dibit_p);
    void processSlotTypePDU();

    DSDDecoder *m_dsdDecoder;
    int  m_symbolIndex;                   //!< current symbol index in non HD sequence
    DSDDMRBurstType m_burstType;
    DSDDMRSlot m_slot;
    unsigned char m_lcss;
    unsigned char m_colorCode;
    DSDDMRDataTYpe m_dataType;
    char *m_slotText;
    int m_slotTextIndex;
    unsigned char m_slotTypePDU_dibits[10];
    Hamming_7_4 m_hamming_7_4;
    Golay_20_8 m_golay_20_8;

    static const int m_cachInterleave[24];
    static const char *m_slotTypeText[13];
};

} // namespace DSDcc



#endif /* DMR_H_ */
