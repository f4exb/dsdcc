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

#include <string.h>
#include "nxdnmessage.h"

namespace DSDcc
{

const unsigned char Message::NXDN_MESSAGE_TYPE_VCALL           = 0x01U;
const unsigned char Message::NXDN_MESSAGE_TYPE_VCALL_IV        = 0x03U;
const unsigned char Message::NXDN_MESSAGE_TYPE_DCALL_HDR       = 0x09U;
const unsigned char Message::NXDN_MESSAGE_TYPE_DCALL_DATA      = 0x0BU;
const unsigned char Message::NXDN_MESSAGE_TYPE_DCALL_ACK       = 0x0CU;
const unsigned char Message::NXDN_MESSAGE_TYPE_TX_REL          = 0x08U;
const unsigned char Message::NXDN_MESSAGE_TYPE_HEAD_DLY        = 0x0FU;
const unsigned char Message::NXDN_MESSAGE_TYPE_SDCALL_REQ_HDR  = 0x38U;
const unsigned char Message::NXDN_MESSAGE_TYPE_SDCALL_REQ_DATA = 0x39U;
const unsigned char Message::NXDN_MESSAGE_TYPE_SDCALL_RESP     = 0x3BU;
const unsigned char Message::NXDN_MESSAGE_TYPE_SDCALL_IV       = 0x3AU;
const unsigned char Message::NXDN_MESSAGE_TYPE_STAT_INQ_REQ    = 0x30U;
const unsigned char Message::NXDN_MESSAGE_TYPE_STAT_INQ_RESP   = 0x31U;
const unsigned char Message::NXDN_MESSAGE_TYPE_STAT_REQ        = 0x32U;
const unsigned char Message::NXDN_MESSAGE_TYPE_STAT_RESP       = 0x33U;
const unsigned char Message::NXDN_MESSAGE_TYPE_REM_CON_REQ     = 0x34U;
const unsigned char Message::NXDN_MESSAGE_TYPE_REM_CON_RESP    = 0x35U;
const unsigned char Message::NXDN_MESSAGE_TYPE_IDLE            = 0x10U;
const unsigned char Message::NXDN_MESSAGE_TYPE_AUTH_INQ_REQ    = 0x28U;
const unsigned char Message::NXDN_MESSAGE_TYPE_AUTH_INQ_RESP   = 0x29U;
const unsigned char Message::NXDN_MESSAGE_TYPE_PROP_FORM       = 0x3FU;

const unsigned char Message::NXDN_MESSAGE_TYPE_VCALL_ASSGN     = 0x04U;
const unsigned char Message::NXDN_MESSAGE_TYPE_SRV_INFO        = 0x19U;
const unsigned char Message::NXDN_MESSAGE_TYPE_SITE_INFO       = 0x18U;
const unsigned char Message::NXDN_MESSAGE_TYPE_GRP_REG_REQ_RESP= 0x24U;

void Message::reset()
{
    memset(m_data, 0, 22);
}

void Message::setFromSACCH(int index, const unsigned char *data)
{
    if (index == 0)
    {
        m_data[0] = data[0];
        m_data[1] = data[1];
        m_data[2] = data[2];
    }
    else if (index == 1)
    {
        m_data[2] = (m_data[2] & 0xC0) + (data[0]>>2);
        m_data[3] = ((data[0]&0x03)<<6) + (data[1]>>2);
        m_data[4] = ((data[1]&0x03)<<6) + (data[2]>>2);
    }
    else if (index == 2)
    {
        m_data[4] = (m_data[4] & 0xF0) + (data[0]>>4);
        m_data[5] = ((data[0]&0x0F)<<4) + (data[1]>>4);
        m_data[6] = ((data[1]&0x0F)<<4) + (data[2]>>4);
    }
    else if (index == 3)
    {
        m_data[6] = (m_data[6] & 0xFC) + (data[0]>>6);
        m_data[7] = ((data[0]&0x3F)<<2) + (data[1]>>6);
        m_data[8] = ((data[1]&0x3F)<<2) + (data[2]>>6);
    }
}

void Message::setFromFACCH1(const unsigned char *data)
{
    memcpy(m_data, data, 10);
}

void Message::setFromFACCH2(const unsigned char *data)
{
    memcpy(m_data, data, 22);
}

void Message::setFromCAC(const unsigned char *data)
{
    memcpy(m_data, data, 17);
}

void Message::setFromCACShort(const unsigned char *data)
{
    memcpy(m_data, data, 11);
}

void Message::setFromCACLong(const unsigned char *data)
{
    memcpy(m_data, data, 15);
}

bool Message::hasCallDetails() const
{
    bool ret;
    switch(getMessageType())
    {
    case NXDN_MESSAGE_TYPE_VCALL:
    case NXDN_MESSAGE_TYPE_DCALL_HDR:
    case NXDN_MESSAGE_TYPE_DCALL_ACK:
    case NXDN_MESSAGE_TYPE_TX_REL:
    case NXDN_MESSAGE_TYPE_HEAD_DLY:
    case NXDN_MESSAGE_TYPE_SDCALL_REQ_HDR:
    case NXDN_MESSAGE_TYPE_SDCALL_RESP:
    case NXDN_MESSAGE_TYPE_STAT_INQ_REQ:
    case NXDN_MESSAGE_TYPE_STAT_INQ_RESP:
    case NXDN_MESSAGE_TYPE_STAT_REQ:
    case NXDN_MESSAGE_TYPE_STAT_RESP:
    case NXDN_MESSAGE_TYPE_REM_CON_REQ:
    case NXDN_MESSAGE_TYPE_REM_CON_RESP:
    case NXDN_MESSAGE_TYPE_AUTH_INQ_REQ:   // group indicator missing
    case NXDN_MESSAGE_TYPE_AUTH_INQ_RESP:  // group indicator missing
        ret = true;
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

bool Message::hasGroupCallInfo() const
{
    bool ret;
    switch(getMessageType())
    {
    case NXDN_MESSAGE_TYPE_VCALL:
    case NXDN_MESSAGE_TYPE_DCALL_HDR:
    case NXDN_MESSAGE_TYPE_DCALL_ACK:
    case NXDN_MESSAGE_TYPE_TX_REL:
    case NXDN_MESSAGE_TYPE_HEAD_DLY:
    case NXDN_MESSAGE_TYPE_SDCALL_REQ_HDR:
    case NXDN_MESSAGE_TYPE_SDCALL_RESP:
    case NXDN_MESSAGE_TYPE_STAT_INQ_REQ:
    case NXDN_MESSAGE_TYPE_STAT_INQ_RESP:
    case NXDN_MESSAGE_TYPE_STAT_REQ:
    case NXDN_MESSAGE_TYPE_STAT_RESP:
    case NXDN_MESSAGE_TYPE_REM_CON_REQ:
    case NXDN_MESSAGE_TYPE_REM_CON_RESP:
        ret = true;
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

unsigned char Message::getMessageType() const
{
    return m_data[0U] & 0x3FU;
}

bool Message::getSourceUnitId(unsigned short& id) const
{
    if (hasCallDetails())
    {
        id = (m_data[3U] << 8) | m_data[4U];
        return true;
    }
    else
    {
        return false;
    }
}

bool Message::getDestinationGroupId(unsigned short& id) const
{
    if (hasCallDetails())
    {
        id = (m_data[5U] << 8) | m_data[6U];
        return true;
    }
    else
    {
        return false;
    }
}

bool Message::isGroupCall(bool& sw) const
{
    if (hasGroupCallInfo())
    {
        sw = (m_data[2U] & 0x80U) != 0x80U;
        return true;
    }
    else
    {
        return false;
    }
}

bool Message::getLocationId(unsigned int& id) const
{
    bool ret;
    switch(getMessageType())
    {
    case NXDN_MESSAGE_TYPE_SITE_INFO:
        id = (m_data[1]<<16) | (m_data[2]<<8) | m_data[3];
        ret = true;
        break;
    case NXDN_MESSAGE_TYPE_SRV_INFO:
        id = (m_data[1]<<16) | (m_data[2]<<8) | m_data[3];
        ret = true;
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

bool Message::getServiceInformation(unsigned short& sibits) const
{
    bool ret;
    switch(getMessageType())
    {
    case NXDN_MESSAGE_TYPE_SITE_INFO:
        sibits = (m_data[6]<<8) | m_data[7];
        ret = true;
        break;
    case NXDN_MESSAGE_TYPE_SRV_INFO:
        sibits = (m_data[4]<<8) | m_data[5];
        ret = true;
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

} // namespace


