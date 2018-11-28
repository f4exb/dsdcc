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

#ifndef NXDN_H_
#define NXDN_H_

#include "pn.h"
#include "viterbi5.h"
#include "nxdnmessage.h"
#include "export.h"

namespace DSDcc
{

class DSDDecoder;

class DSDCC_API DSDNXDN
{
public:
    typedef enum
    {
    	NXDNFrame,
		NXDNPostFrame,
		NXDNSwallow
    } NXDNState;

    typedef enum
    {
        NXDNRCCH,
        NXDNRTCH,
        NXDNRDCH,
        NXDNRTCHC,
        NXDNRFCHUnknown
    } NXDNRFChannel;

    typedef enum
    {
        NXDNFSReserved,
        NXDNFSCAC,       // RCCH CAC outbound
        NXDNFSCACShort,  // RCCH short CAC
        NXDNFSCACLong,   // RCCH long CAC
        NXDNFSSACCH,     // RTCH,RDCH SACCH single
        NXDNFSSACCHSup,  // RTCH,RDCH SACCH superframe
        NXDNFSSACCHIdle, // RTCH,RDCH SACCH superframe idle
        NXDNFSUDCH,      // RTCH,RDCH UDCH
    } NXDNFrameStructure;

    typedef enum
    {
        NXDNStealBoth,   // All steal: 2xFACCH1 (SACCH) or FACCH2 (UDCH)
        NXDNSteal1,      // steal VCH first:  2xVCH + FACCH1 (only SACCH)
        NXDNSteal2,      // steal VCH second: FACCH1 + 2xVCH (only SACCH)
        NXDNStealNone,   // No steal: VCH (SACCH) or UDCH (UDCH) only
        NXDNStealReserved,
    } NXDNSteal;

	explicit DSDNXDN(DSDDecoder *dsdDecoder);
	~DSDNXDN();

    void init();
    void process();

    NXDNRFChannel getRFChannel() const { return m_rfChannel; }
    const char *getRFChannelStr() const { return m_rfChannelStr; }
    bool isIdle() const { return m_idle; }
    int getRAN() const { return m_ran; }
    unsigned char getMessageType() const { return m_messageType; }
    unsigned short getSourceId() const { return m_sourceId; }
    unsigned short getDestinationId() const { return m_destinationId; }
    bool isGroupCall() const { return m_group; }
    unsigned int getLocationId() const { return m_locationId; }
    unsigned int getServicesFlag() const { return m_services; }
    bool isFullRate() const;

    static const char *nxdnRFChannelTypeText[5];

private:
    struct NXDNLICH
    {
        NXDNLICH() :
            rfChannelCode(0),
            fnChannelCode(0),
            optionCode(0),
            direction(0),
            parity(0)
        {}

    	int rfChannelCode; //!< LICH RF channel type code (should always be 2 as RDCH)
    	int fnChannelCode; //!< LICH Functional channel type code
    	int optionCode;    //!< LICH Option code
    	int direction;     //!< LICH direction code: 0: inbound (MS), 1: outbound (BS)
    	int parity;        //!< LICH bits even parity
    };

    class FnChannel
    {
    public:
        FnChannel();
        virtual ~FnChannel();
        void reset();
        void pushDibit(unsigned char dibit);
        void unpuncture();
        virtual bool decode() = 0;
    protected:
        int m_index;
        int m_nbPuncture;
        int m_rawSize;
        unsigned char *m_bufRaw;
        unsigned char *m_bufTmp;
        const int *m_interleave;
        const int *m_punctureList;
    };

    class SACCH : public FnChannel
    {
    public:
        SACCH();
        virtual ~SACCH();
        virtual bool decode();
        unsigned char getRAN() const;
        unsigned char getCountdown() const;  //!< get SACCH block countdown in a superframe structure
        int getDecodeCount() const { return m_decodeCount; }
        const Message& getMessage() const { return m_message; }
        static const int m_Interleave[60];   //!< SACCH bits interleaving matrix
        static const int m_PunctureList[12]; //!< SACCH punctured bits indexes
    private:
        unsigned char m_sacchRaw[60];        //!< SACCH bits retrieved from RF channel
        unsigned char m_temp[90];            //!< SACCH working area;
        unsigned char m_data[5];             //!< SACCH bytes after de-convolution (36 bits)
        Message m_message;                   //!< locally stored layer-3 message
        int m_decodeCount;                   //!< count of subsequent successful decodes starting at start of superframe
    };

    class CACOutbound : public FnChannel
    {
    public:
        CACOutbound();
        virtual ~CACOutbound();
        virtual bool decode();
        unsigned char getRAN() const;
        bool isHeadOfSuperframe() const;
        bool hasDualMessageFormat() const;
        unsigned char getMessageType() const;
        const unsigned char *getData() const { return m_data; }
        static const int m_Interleave[300];  //!< CAC outbound bits interleaving matrix
        static const int m_PunctureList[50]; //!< CAC outbound punctured bits indexes
    private:
        unsigned char m_cacRaw[300];            //!< CAC outbound bits before Viterbi decoding
        unsigned char m_temp[420];              //!< CAC outbound working area
        unsigned char m_data[22];               //!< CAC outbound bytes after de-convolution (175 bits)
    };

    class CACLong : public FnChannel
    {
    public:
        CACLong();
        virtual ~CACLong();
        virtual bool decode();
        unsigned char getRAN() const;
        const unsigned char *getData() const { return m_data; }
        static const int m_Interleave[252];  //!< Long CAC bits interleaving matrix
        static const int m_PunctureList[60]; //!< Long CAC punctured bits indexes
    private:
        unsigned char m_cacRaw[252];            //!< Long CAC bits before Viterbi decoding
        unsigned char m_temp[420];              //!< Long CAC working area
        unsigned char m_data[20];               //!< Long CAC bytes after de-convolution (156 bits)
    };

    class CACShort : public FnChannel
    {
    public:
        CACShort();
        virtual ~CACShort();
        virtual bool decode();
        unsigned char getRAN() const;
        const unsigned char *getData() const { return m_data; }
    private:
        unsigned char m_cacRaw[252];           //!< Short CAC bits before Viterbi decoding
        unsigned char m_temp[420];             //!< Short CAC working area
        unsigned char m_data[16];              //!< Short CAC bytes after de-convolution (126 bits)
    };

    class FACCH1 : public FnChannel
    {
    public:
        FACCH1();
        virtual ~FACCH1();
        virtual bool decode();
        const unsigned char *getData() const { return m_data; }
        static const int m_Interleave[144];     //!< FACCH1 bits interleaving matrix
        static const int m_PunctureList[48];    //!< FACCH1 punctured bits indexes
    private:
        unsigned char m_facch1Raw[192];         //!< FACCH1 bits before Viterbi decoding
        unsigned char m_temp[210];              //!< FACCH1 working area
        unsigned char m_data[12];               //!< FACCH1 bytes after de-convolution (96 bits)
    };

    class UDCH : public FnChannel
    {
    public:
        UDCH();
        virtual ~UDCH();
        virtual bool decode();
        unsigned char getRAN() const;
        unsigned char getStructure() const;     //!< get (super)frame structure information (2 bits)
        const unsigned char *getData() const { return m_data; }
        static const int m_Interleave[348];     //!< UDCH bits interleaving matrix
        static const int m_PunctureList[58];    //!< UDCH punctured bits indexes
    private:
        unsigned char m_udchRaw[406];           //!< UDCH bits before Viterbi decoding
        unsigned char m_temp[420];              //!< UDCH working area
        unsigned char m_data[26];               //!< UDCH bytes after de-convolution (203 bits)
    };

    int unscrambleDibit(int dibit);
    void processFrame();
    void processPostFrame();
    void acquireLICH(int dibit);
    void processLICH();
    void processFSW();
    void processSwallow();
    void processRCCH(int index, unsigned char dibit);
    void processRTDCH(int index, unsigned char dibit);
    void processFACCH1(int index, unsigned char dibit);

    void processVoiceTest(int symbolIndex);
    void processVoiceFrameEHR(int symbolIndex, int dibit);
    void processVoiceFrameEFR(int symbolIndex, int dibit);
    void storeSymbolDV(int dibitindex, unsigned char dibit, bool invertDibit = false);
    void resetAdjacentSites();
    void printAdjacentSites();

	DSDDecoder *m_dsdDecoder;
	NXDNState   m_state;
	NXDNLICH    m_lich;             //!< Link Information CHannel data (LICH)
	PN_9_5      m_pn;
	bool        m_inSync;           //!< used to notify when entering into NXDN sync state
	unsigned char m_syncBuffer[10]; //!< buffer for frame sync: 10  dibits
	unsigned char m_lichBuffer[8];  //!< LICH bits expanded to char (0 or 1)
	int m_lichEvenParity;           //!< Even parity bits count for LICH
	int m_symbolIndex;              //!< current symbol index in non HD sequence
	int m_swallowCount;             //!< count of symbols to swallow (used in swallow state)
    NXDNRFChannel m_rfChannel;      //!< current RF channel type (from LICH)
    NXDNFrameStructure m_frameStructure; //!< frame structure indicator
    NXDNSteal m_steal;              //!< stealing scheme
    unsigned char m_messageType;    //!< Latest message type
    int m_ran;                      //!< Radio Access Number (kind of color code)
    bool m_idle;                    //!< Channel in idle state
    unsigned short m_sourceId;      //!< Latest source Id
    unsigned short m_destinationId; //!< Latest source Id
    bool m_group;                   //!< Latest group indicator
    unsigned int m_locationId;      //!< Site location Id (trunked)
    unsigned short m_services;      //!< Site services available (trunked)
    bool m_fullRate;                //!< Full rate vocoder (EFR) else half rate (EHR)

    CACOutbound m_cac;
    CACShort m_cacShort;
    CACLong m_cacLong;
    SACCH m_sacch;
    FACCH1 m_facch1;
    UDCH m_udch;
    Message m_currentMessage;
    AdjacentSiteInformation m_adjacentSites[16];

    char m_rfChannelStr[2+1];

    static const int rW[36];
    static const int rX[36];
    static const int rY[36];
    static const int rZ[36];
    const int *w, *x, *y, *z;
    static const unsigned char m_voiceTestPattern[36];
};

} // namespace DSDcc

#endif /* NXDN_H_ */
