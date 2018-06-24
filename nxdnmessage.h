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

#ifndef NXDNMESSAGE_H_
#define NXDNMESSAGE_H_

namespace DSDcc
{

struct AdjacentSiteInformation
{
    unsigned char m_siteNumber;      // 1 to 16
    unsigned int m_locationId;       // 24 bit
    unsigned short m_channelNumber;  // 10 bit
};

/** A layer-3 message */
struct Message
{
public:
    void reset();
    void setMessageIndex(unsigned int index); //!< sets the message index in dual message case
    void setFromSACCH(int index, const unsigned char *data);
    void setFromFACCH1(const unsigned char *data);
    void setFromFACCH2(const unsigned char *data);
    void setFromCAC(const unsigned char *data);
    void setFromCACShort(const unsigned char *data);
    void setFromCACLong(const unsigned char *data);
    bool hasBroadcastInformation() const;
    unsigned char  getMessageType() const;
    bool getSourceUnitId(unsigned short& id) const;
    bool getDestinationGroupId(unsigned short& id) const;
    bool isGroupCall(bool& sw) const;
    bool getLocationId(unsigned int& id) const;
    bool getServiceInformation(unsigned short& sibits) const;
    bool getAdjacentSitesInformation(AdjacentSiteInformation *adjacentSites, int nbSitesToGet) const;
    bool isFullRate(bool& fullRate) const;

    static const unsigned char NXDN_MESSAGE_TYPE_VCALL;
    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_IV;
    static const unsigned char NXDN_MESSAGE_TYPE_DCALL_HDR;
    static const unsigned char NXDN_MESSAGE_TYPE_DCALL_DATA;
    static const unsigned char NXDN_MESSAGE_TYPE_DCALL_ACK;
    static const unsigned char NXDN_MESSAGE_TYPE_TX_REL;
    static const unsigned char NXDN_MESSAGE_TYPE_HEAD_DLY;
    static const unsigned char NXDN_MESSAGE_TYPE_SDCALL_REQ_HDR;
    static const unsigned char NXDN_MESSAGE_TYPE_SDCALL_REQ_DATA;
    static const unsigned char NXDN_MESSAGE_TYPE_SDCALL_RESP;
    static const unsigned char NXDN_MESSAGE_TYPE_SDCALL_IV;
    static const unsigned char NXDN_MESSAGE_TYPE_STAT_INQ_REQ;
    static const unsigned char NXDN_MESSAGE_TYPE_STAT_INQ_RESP;
    static const unsigned char NXDN_MESSAGE_TYPE_STAT_REQ;
    static const unsigned char NXDN_MESSAGE_TYPE_STAT_RESP;
    static const unsigned char NXDN_MESSAGE_TYPE_REM_CON_REQ;
    static const unsigned char NXDN_MESSAGE_TYPE_REM_CON_RESP;
    static const unsigned char NXDN_MESSAGE_TYPE_IDLE;
    static const unsigned char NXDN_MESSAGE_TYPE_AUTH_INQ_REQ;
    static const unsigned char NXDN_MESSAGE_TYPE_AUTH_INQ_RESP;
    static const unsigned char NXDN_MESSAGE_TYPE_PROP_FORM;

    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_ASSGN;
    static const unsigned char NXDN_MESSAGE_TYPE_SRV_INFO;
    static const unsigned char NXDN_MESSAGE_TYPE_SITE_INFO;
    static const unsigned char NXDN_MESSAGE_TYPE_ADJ_SITE_INFO;
    static const unsigned char NXDN_MESSAGE_TYPE_GRP_REG_REQ_RESP;

    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_REQ;
    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_RESP;
    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_REC_REQ;
    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_REC_RESP;
    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_CONN_REQ;
    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_CONN_RESP;
    static const unsigned char NXDN_MESSAGE_TYPE_VCALL_ASSGN_DUP;

private:
    bool hasCallDetails() const;
    bool hasGroupCallInfo() const;
    unsigned char m_data[22];  //!< Maximum 22 bytes
    unsigned int m_shift;      //!< index shift for dual messages
};

} // namespace

#endif /* NXDNMESSAGE_H_ */
