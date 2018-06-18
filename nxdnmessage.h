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

/** A layer-3 message */
struct Message
{
public:
    void reset();
    void setFromSACCH(int index, const unsigned char *data);
    void setFromFACCH1(const unsigned char *data);
    void setFromFACCH2(const unsigned char *data);
    bool hasCallDetails() const;
    unsigned char  getMessageType() const;
    unsigned short getSourceUnitId() const;
    unsigned short getDestinationGroupId() const;
    bool           getIsGroup() const;

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
    static const unsigned char NXDN_MESSAGE_TYPE_GRP_REG_REQ_RESP;

private:
    unsigned char m_data[22];               //!< Maximum 22 bytes
};

} // namespace

#endif /* NXDNMESSAGE_H_ */
