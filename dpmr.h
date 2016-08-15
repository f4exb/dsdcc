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

#ifndef DPMR_H_
#define DPMR_H_

namespace DSDcc
{

class DSDDecoder;

class DSDdPMR
{
public:
    typedef enum
    {
    	DPMRNoFrame,         //!< no sync
        DPMRExtSearchFrame,  //!< no sync - extensive search
    	DPMRHeaderFrame,     //!< header
		DPMRPayloadFrame,    //!< payload superframe with undetermined type
        DPMRVoiceframe,      //!< voice superframe (no SLD)
        DPMRDataVoiceframe,  //!< data and voice superframe (type 2)
        DPMRData1frame,      //!< data superframe wihout FEC
        DPMRData2frame,      //!< data superframe with FEC
		DPMREndFrame,        //!< end frame
    } DPMRFrameType;

    typedef enum
    {
        DPMRCommStartHeader, //!< Communication start header (a superframe follows)
        DPMRConnReqHeader,   //!< Connection request header (an END frame follows)
        DPMRUnConnReqHeader, //!< Unconnect request header (an END frame follows)
        DPMRAckHeader,       //!< ACK (this a single frame, ACK or NACK is differentiated by the CI bits setting)
        DPMRSysReqHeader,    //!< System request header (an END frame follows)
        DPMRAckReplyHeader,  //!< ACK header reply to a system request (a superframe follows)
        DPMRSysDelivHeader,  //!< System delivery header (a superframe follows)
        DPMRStatRespHeader,  //!< Status response header (an END frame follows)
        DPMRStatReqHeader,   //!< Status request header
        DPMRReservedHeader,  //!< Reserved
        DPMRUndefinedHeader  //!< Undefined
    } DPMRHeaderType;

    typedef enum
    {
        DPMRVoiceMode,       //!< Voice communication (no user data in SLD field)
        DPMRVoiceSLDMode,    //!< Voice + slow data (user data in SLD field)
        DPMRData1Mode,       //!< Data communication type 1 (Payload is user data without FEC)
        DPMRData2Mode,       //!< Data communication type 2 (Payload is user data with FEC)
        DPMRData3Mode,       //!< Data communication type 3 (Packet data, ARQ method)
        DPMRVoiceDataMode,   //!< Voice and appended data (Type 2)
        DPMRReservedMode,    //!< Reserved
        DPMRUndefinedMode    //!< Undefined
    } DPMRCommMode;

    typedef enum
    {
        DPMRCallAllFormat,   //!< Call ALL (Broadcast)
        DPMRP2PFormat,       //!< Peer-to-peer communication
        DPMRReservedFormat,  //!< Reserved
        DPMRUndefinedFormat  //!< Undefined
    } DPMRCommFormat;

    DSDdPMR(DSDDecoder *dsdDecoder);
    ~DSDdPMR();

    void init();
    void process();

    int getColorCode() const { return m_colourCode; }
    DPMRFrameType getFrameType() const { return m_frameType; }
    unsigned int getCalledId() const { return m_calledId; }
    unsigned int getOwnId() const { return m_ownId; }

private:
    class LFSRGenerator
    {
    public:
        LFSRGenerator();
        ~LFSRGenerator();

        void init();
        unsigned int next();

    private:
        unsigned int m_sr;
    };

    class Hamming_12_8
    {
    public:
        Hamming_12_8();
        ~Hamming_12_8();

        void init();
        bool decode(unsigned char *rxBits, unsigned char *decodedBits, int nbCodewords);

    private:
        unsigned char m_corr[16];                       //!< single bit error correction by syndrome index
        static const unsigned char m_H[12*4]; //!< Parity check matrix of bits
    };

    typedef enum
    {
       DPMRHeader,         // FS1 sync header frame (sync detected at upper level)
       DPMRPostFrame,      // frame(s) have been processed and we are looking for a FS2 or FS3 sync
       DPMRExtSearch,      // FS2 or FS3 extensive search
       DPMRSuperFrame,     // process superframe
       DPMREnd             // FS3 sync end frame
    } DPMRState;

    void processHeader();
    void processHIn(int symbolIndex, int dibit);
    void processSuperFrame(); // process super frame
    void processEndFrame();
    void processPostFrame();
    void processExtSearch();
    void processColourCode(int symbolIndex, int dibit);
    void processFS2(int symbolIndex, int dibit);
    void processCCH(int symbolIndex, int dibit);
    void processTCH(int symbolIndex, int dibit);
    void processVoiceFrame(int symbolIndex, int dibit);
    void storeSymbolDV(int dibitindex, unsigned char dibit, bool invertDibit = false);
    void initScrambling();
    void initInterleaveIndexes();
    bool checkCRC7(unsigned char *bits, int nbBits);
    bool checkCRC8(unsigned char *bits, int nbBits);

    DSDDecoder *m_dsdDecoder;
    DPMRState   m_state;
    DPMRFrameType m_frameType;
    unsigned char m_syncDoubleBuffer[24]; //!< double buffer for frame sync extensive search
    unsigned char  m_colourBuffer[12];    //!< buffer for colour code: 12  dibits
    int  m_syncCycle;
    int  m_symbolIndex;                   //!< current symbol index in non HD sequence
    unsigned int  m_frameIndex;           //!< count of frames in superframes with best effort
    int  m_colourCode;                    //!< calculated colour code
    LFSRGenerator m_scramblingGenerator;
    Hamming_12_8  m_hamming;
    unsigned char m_scrambleBits[120];
    unsigned char m_bitBufferRx[120];
    unsigned char m_bitBuffer[80];
    unsigned char m_bitWork[80];
    unsigned int dI72[72];
    unsigned int dI120[120];
    DPMRHeaderType m_headerType;
    DPMRCommMode m_commMode;
    DPMRCommFormat m_commFormat;
    unsigned int m_calledId;
    unsigned int m_ownId;
    unsigned char m_frameNumber;

    static const int rW[36];
    static const int rX[36];
    static const int rY[36];
    static const int rZ[36];
    static const unsigned char m_fs2[12];
    static const unsigned char m_fs3[12];
    static const unsigned char m_preamble[12];
    const int *w, *x, *y, *z;
};


} // namespace DSDcc

#endif /* DPMR_H_ */
