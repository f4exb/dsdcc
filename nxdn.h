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

namespace DSDcc
{

class DSDDecoder;

class DSDNXDN
{
public:
	explicit DSDNXDN(DSDDecoder *dsdDecoder);
	~DSDNXDN();

    void init();
    void process();

private:
    typedef enum
    {
    	NXDNFrame,
		NXDNPostFrame,
		NXDNSwallow
    } NXDNState;

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

    void processFrame();
    void processPostFrame();
    void processLICH();
    void processFSW();
    void processSwallow();

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
};

} // namespace DSDcc

#endif /* NXDN_H_ */
