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

#ifndef YSF_H_
#define YSF_H_

#include <iostream>
#include <string>

#include "viterbi5.h"
#include "fec.h"
#include "crc.h"

namespace DSDcc
{

class DSDDecoder;

class DSDYSF
{
public:
    typedef enum
    {
        FIHeader,
        FICommunication,
        FITerminator,
        FITest
    } FrameInformation;

    typedef enum
    {
        CMGroupCQ,
        CMRadioID,
        CMReserved,
        CMIndividual
    } CallMode;

    typedef enum
    {
        MRDirectWave,
        MRDownlinUplinkkNotBusy,
        MRDownlinkUplinkBusy,
        MRReserved
    } MessageRouting;

    typedef enum
    {
        DTVoiceData1,
        DTDataFullRate,
        DTVoiceData2,
        DTVoiceFullRate
    } DataType;

    typedef enum
    {
    	FICHNoError,
		FICHErrorGolay,
		FICHErrorCRC
    } FICHError;

    struct FICH
    {
        uint8_t m_frameInfo[2];     //!< 31:30 FI
        uint8_t m_callsignType[2];  //!< 29:28 CS
        uint8_t m_callMode[2];      //!< 27:26 CM
        uint8_t m_blockNumber[2];   //!< 25:24 BN
        uint8_t m_blockTotal[2];    //!< 23:22 BT
        uint8_t m_frameNumber[3];   //!< 21:19 FN
        uint8_t m_frameTotal[3];    //!< 18:17 FT
        uint8_t m_reserved;         //!< 15
        uint8_t m_freqDeviation;    //!< 14    Dev
        uint8_t m_messagePath[3];   //!< 13:11 MR
        uint8_t m_voipPath;         //!< 10    VoIP
        uint8_t m_dataType[2];      //!< 9:8 DT
        uint8_t m_sqlType;          //!< 7     SQL
        uint8_t m_sqlCode[7];       //!< 6:0   SC

        FrameInformation getFrameInformation() const {
            return (FrameInformation) (((m_frameInfo[0]&1)<<1) + (m_frameInfo[1]&1));
        }

        CallMode getCallMode() const {
            return (CallMode) (((m_callMode[0]&1)<<1) + (m_callMode[1]&1));
        }

        int getBlockNumber() const {
            return ((m_blockNumber[0]&1)<<1) + (m_blockNumber[1]&1);
        }

        int getBlockTotal() const {
            return ((m_blockTotal[0]&1)<<1) + (m_blockTotal[1]&1);
        }

        int getFrameNumber() const {
            return ((m_frameNumber[0]&1)<<2) + ((m_frameNumber[1]&1)<<1) + (m_frameNumber[2]&1);
        }

        int getFrameTotal() const {
            return ((m_frameTotal[0]&1)<<2) + ((m_frameTotal[1]&1)<<1) + (m_frameTotal[2]&1);
        }

        bool isNarrowMode() const {
            return (m_freqDeviation & 1) == 1;
        }

        MessageRouting getMessageRouting() const {
            int mrValue = ((m_messagePath[0]&1)<<2) + ((m_messagePath[1]&1)<<1) + (m_messagePath[2]&1);

            if (mrValue < (int) MRReserved) {
                return (MessageRouting) mrValue;
            } else {
                return MRReserved;
            }
        }

        bool isInternetPath() const {
            return (m_voipPath & 1) == 1;
        }

        DataType getDataType() const {
            return (DataType) (((m_dataType[0]&1)<<1) + (m_dataType[1]&1));
        }

        bool isSquelchCodeEnabled() const {
            return (m_sqlType & 1) == 0;
        }

        int getSquelchCode() const {
            return ((m_sqlCode[0]&1)<<6)
                    + ((m_sqlCode[1]&1)<<5)
                    + ((m_sqlCode[2]&1)<<4)
                    + ((m_sqlCode[3]&1)<<3)
                    + ((m_sqlCode[4]&1)<<2)
                    + ((m_sqlCode[5]&1)<<1)
                    + (m_sqlCode[6]&1);
        }

        friend std::ostream &operator<<(std::ostream &output, const FICH& fich)
        {
            output << "FI: " << (int) fich.getFrameInformation()
                    << " CM: " << (int) fich.getCallMode()
                    << " Block#: " << fich.getBlockNumber() << "/" << fich.getBlockTotal()
                    << " Frame#: " << fich.getFrameNumber() << "/" << fich.getFrameTotal()
                    << " Dev: " << (fich.isNarrowMode() ? "narrow" : "wide")
                    << " MR: " << (int) fich.getMessageRouting()
                    << " VoIP: " << (fich.isInternetPath() ? "internet" : "local")
                    << " DT: " << (int) fich.getDataType()
                    << " SQL: " << (fich.isSquelchCodeEnabled() ? "on" : "off")
                    << " SQL code: " << fich.getSquelchCode();
            return output;
        }
    };

    DSDYSF(DSDDecoder *dsdDecoder);
    ~DSDYSF();

    void init();
    void process();

    const FICH& getFICH() const { return m_fich; }
    FICHError getFICHError() const { return m_fichError; }

private:

    void processFICH(int symbolIndex, unsigned char dibit);
    bool checkCRC16(unsigned char *bits, unsigned long nbBytes);

    DSDDecoder *m_dsdDecoder;
    int m_symbolIndex;                //!< Current symbol index
    unsigned char m_fichRaw[100];     //!< FICH dibits after de-interleave + Viterbi stuff symbols
    unsigned char m_fichGolay[100];   //!< FICH Golay encoded bits + 4 stuff bits + Viterbi stuff bits
    unsigned char m_fichBits[48];     //!< Final FICH + CRC16
    unsigned char m_fichCRC[16];      //!< Calculated CRC
    FICH          m_fich;             //!< Validated FICH
    FICHError     m_fichError;        //!< FICH decoding error status
    Viterbi5 m_viterbiFICH;
    Golay_24_12 m_golay_24_12;
    CRC m_crc;
    unsigned char m_bitWork[48];

    static const int m_fichInterleave[100]; //!< FICH symbols interleaving matrix
};

} // namespace DSDcc



#endif /* YSF_H_ */
