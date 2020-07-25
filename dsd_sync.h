///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2020 Edouard Griffiths, F4EXB.                                  //
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

#ifndef DSD_SYNC_H_
#define DSD_SYNC_H_

#include "export.h"

namespace DSDcc
{

class DSDCC_API DSDSync
{
public:
    enum SyncPattern
    {
        SyncDMRDataBS,
        SyncDMRVoiceBS,
        SyncDMRDataMS,
        SyncDMRVoiceMS,
        SyncDPMRFS1,
        SyncDPMRFS4,
        SyncDPMRFS2,
        SyncDPMRFS3,
        SyncNXDNRDCHFull,
        SyncNXDNRDCHFullInv,
        SyncNXDNRDCHFSW,
        SyncNXDNRDCHFSWInv,
        SyncDStarHeader,
        SyncDStarHeaderInv,
        SyncDStar,
        SyncDStarInv,
        SyncYSF,
        SyncP25P1,
        SyncP25P1Inv,
        SyncX2TDMADataBS,
        SyncX2TDMAVoiceBS,
        SyncX2TDMADataMS,
        SyncX2TDMAVoiceMS,
        SyncProVoice,
        SyncProVoiceInv,
        SyncProVoiceEA,
        SyncProVoiceEAInv
    };

    static const int m_history = 32;
    static const int m_patterns = 27;
    static const unsigned char m_syncPatterns[m_patterns][m_history]; //!< Patterns
    static const unsigned int m_syncLenTol[m_patterns][2];            //!< Length (0) and tolerance (1)
    unsigned int m_syncErrors[m_patterns];

    static const unsigned char *getPattern(SyncPattern pattern, int& length);
    void matchAll(const unsigned char *start);
    void matchSome(const unsigned char *start, int maxHistory, const SyncPattern *patterns, int nbPatterns);
    bool isMatching(SyncPattern pattern);
    unsigned int getErrors(SyncPattern pattern);
};

} // namespace DSDcc

#endif // DSD_SYNC_H_