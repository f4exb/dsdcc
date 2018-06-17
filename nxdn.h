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

namespace DSDcc
{

class DSDDecoder;

class DSDNXDN
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
        NXDNRFCHUnknown
    } NXDNRFChannel;

	explicit DSDNXDN(DSDDecoder *dsdDecoder);
	~DSDNXDN();

    void init();
    void process();

    const char *getRfChannel() const { return m_rfChannelStr; }

    static const char *nxdnRFChannelTypeText[4];

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
        virtual void decode() = 0;
    protected:
        int m_index;
        Viterbi5 m_viterbi;
        int m_nbPuncture;
        int m_rawSize;
        unsigned char *m_bufRaw;
        unsigned char *m_buf;
        const int *m_interleave;
        const int *m_punctureList;
    };

    class SACCH : public FnChannel
    {
    public:
        SACCH();
        virtual ~SACCH();
        virtual void decode();
        static const int m_sacchInterleave[60];   //!< SACCH bits interleaving matrix
        static const int m_sacchPunctureList[12]; //!< SACCH punctured bits indexes
    private:
        unsigned char m_sacchRaw[72];             //!< SACCH bits before Viterbi decoding
        unsigned char m_sacch[36];                //!< SACCH bits
    };

    class CACOutbound : public FnChannel
    {
    public:
        CACOutbound();
        virtual ~CACOutbound();
        virtual void decode();
        static const int m_cacInterleave[300];  //!< CAC outbound bits interleaving matrix
        static const int m_cacPunctureList[50]; //!< CAC outbound punctured bits indexes
    private:
        unsigned char m_cacRaw[350];            //!< CAC outbound bits before Viterbi decoding
        unsigned char m_cac[175];               //!< CAC outbound bits
    };

    class CACLong : public FnChannel
    {
    public:
        CACLong();
        virtual ~CACLong();
        virtual void decode();
        static const int m_cacInterleave[252];  //!< Long CAC bits interleaving matrix
        static const int m_cacPunctureList[60]; //!< Long CAC punctured bits indexes
    private:
        unsigned char m_cacRaw[312];            //!< Long CAC bits before Viterbi decoding
        unsigned char m_cac[156];               //!< Long CAC bits
    };

    class CACShort : public FnChannel
    {
    public:
        CACShort();
        virtual ~CACShort();
        virtual void decode();
    private:
        unsigned char m_cacRaw[252];           //!< Short CAC bits before Viterbi decoding
        unsigned char m_cac[126];              //!< Short CAC bits
    };

    int unscrambleDibit(int dibit);
    void processFrame();
    void processPostFrame();
    void acquireLICH(int dibit);
    void processLICH();
    void processFSW();
    void processSwallow();
    void processRCCH(int index, unsigned char dibit);
    void processRTCH(int index, unsigned char dibit);
    void processRDCH(int index, unsigned char dibit);

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


    char m_rfChannelStr[2+1];

};

} // namespace DSDcc

#endif /* NXDN_H_ */
