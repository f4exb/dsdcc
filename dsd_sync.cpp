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

#include <algorithm>
#include "dsd_sync.h"

namespace DSDcc
{

const unsigned char DSDSync::m_syncPatterns[27][32] = {
//  <--- past
//   3  3  2  2  2  2  2  2  2  2  2  2  1  1  1  1  1  1  1  1  1  1
//   1  0  9  9  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0
//  index --->
//                                 1  1  1  1  1  1  1  1  1  1  2  2  2  2  2  2  2  2  2  2  3  3
//   0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 3, 3, 3, 1, 1, 1, 3, 3, 1, 1, 3, 1, 1, 3, 1, 3, 3, 1, 1, 3, 1}, //  0: SyncDMRDataBS:  DF F5 7D 75 DF 5D
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 1, 3, 3, 3, 1, 1, 3, 3, 1, 3, 3, 1, 3, 1, 1, 3, 3, 1, 3}, //  1: SyncDMRVoiceBS: 75 5F D7 DF 75 F7
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 3, 1, 1, 3, 3, 3, 1, 3, 1, 3, 3, 3, 3, 1, 1, 3, 1, 1, 1, 3}, //  2: SyncDMRDataMS:  D5 D7 F7 7F D7 57
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 3, 3, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 3, 3, 1, 3, 3, 3, 1}, //  3: SyncDMRVoiceMS: 7F 7D 5D D5 7D FD
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 3, 3, 3, 3, 3, 1, 1, 3, 3, 1, 3, 1, 1, 3, 1, 1, 1, 1, 3, 1, 3}, //  4: SyncDPMRFS1:    57 FF 5F 75 D5 77 - non packet data header
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 1, 1, 1, 1, 1, 3, 3, 1, 1, 3, 1, 3, 3, 1, 3, 3, 3, 3, 1, 3, 1}, //  5: SyncDPMRFS4:    FD 55 F5 DF 7F DD - packet data header
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 3, 3, 3, 1, 3, 1, 3, 3, 1}, //  6: SyncDPMRFS2:    5F F7 7D          - superframe sync (each 2 384 bit frames)
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 1, 3, 1, 3, 3, 3, 3, 1, 1}, //  7: SyncDPMRFS3:    7D DF F5          - end frame sync
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 1, 1, 3, 3, 3, 1, 3, 1, 3, 1, 3, 3, 1, 1, 3, 1}, //  8: SyncNXDNRDCHFull
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 3, 3, 1, 1, 1, 3, 1, 3, 1, 3, 1, 1, 3, 3, 1, 3}, //  9: SyncNXDNRDCHFullInv
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 1, 3, 3, 1, 1, 3, 1}, // 10: SyncNXDNRDCHFSW
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 3, 1, 1, 3, 3, 1, 3}, // 11: SyncNXDNRDCHFSWInv
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 3, 3, 1, 3, 3, 1, 1, 3, 1, 3, 1, 1, 1, 1}, // 12: SyncDStarHeader
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 1, 1, 3, 1, 1, 3, 3, 1, 3, 1, 3, 3, 3, 3}, // 13: SyncDStarHeaderInv
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 3, 1, 3, 1, 1, 1, 3, 3, 1, 3, 1, 1, 1}, // 14: SyncDStar
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 1, 3, 1, 3, 3, 3, 1, 1, 3, 1, 3, 3, 3}, // 15: SyncDStarInv
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 1, 3, 1, 1, 3, 1, 3, 1, 1, 3, 1, 3, 1, 1, 3, 1}, // 16: SyncYSF         D4 71 C9 63 4D => D5 75 DD 77 5D
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 3, 1, 1, 3, 3, 1, 1, 3, 3, 3, 3, 1, 3, 1, 3, 3, 3, 3, 3}, // 17: SyncP25P1
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 1, 3, 3, 1, 1, 3, 3, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1}, // 18: SyncP25P1Inv
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 1, 3, 1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 3, 3, 3, 1, 1, 3, 3}, // 19: SyncX2TDMADataBS
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 1, 3, 1, 3, 3, 3, 3, 3, 1, 3, 1, 3, 3, 3, 1, 1, 1, 3, 3, 1, 1}, // 20: SyncX2TDMAVoiceBS
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 1, 1, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3 ,3 ,3 ,1 ,3}, // 21: SyncX2TDMADataMS
    {0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 1, 1, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3 ,3 ,3 ,1 ,3}, // 22: SyncX2TDMAVoiceMS
    {1, 3, 1, 3, 1, 3, 3, 3, 1, 1, 1, 3, 1, 1, 3, 1, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1, 3, 3}, // 23: SyncProVoice
    {3, 1, 3, 1, 3, 1, 1, 1, 3, 3, 3, 1, 3, 3, 1, 3, 3, 3, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1}, // 24: SyncProVoiceInv
    {3, 1, 1, 3, 1, 3, 1, 1, 3, 3, 1, 3, 3, 1, 1, 1, 1, 1, 3, 3, 1, 3, 1, 3, 1, 1, 3 ,1 ,1 ,1, 3 ,3}, // 25: SyncProVoiceEA
    {1, 3, 3, 1, 3, 1, 3, 3, 1, 1, 3, 1, 1 ,3 ,3, 3, 3 ,3, 1, 1 ,3, 1, 3, 1, 3, 3, 1, 3, 3, 3, 1, 1}  // 26: SyncProVoiceEAInv
};

// First column: number of synchronization symbols
// Second column: tolerance in number of mismatching symbols
const unsigned int DSDSync::m_syncLenTol[27][2] = {
    {24, 2}, //  0: SyncDMRDataBS
    {24, 2}, //  1: SyncDMRVoiceBS
    {24, 2}, //  2: SyncDMRDataMS
    {24, 2}, //  3: SyncDMRVoiceMS
    {24, 2}, //  4: SyncDPMRFS1
    {24, 2}, //  5: SyncDPMRFS4
    {12, 1}, //  6: SyncDPMRFS2
    {12, 1}, //  7: SyncDPMRFS3
    {19, 1}, //  8: SyncNXDNRDCHFull
    {19, 1}, //  9: SyncNXDNRDCHFullInv
    {10, 1}, // 10: SyncNXDNRDCHFSW
    {10, 1}, // 11: SyncNXDNRDCHFSWInv
    {24, 2}, // 12: SyncDStarHeader
    {24, 2}, // 13: SyncDStarHeaderInv
    {24, 2}, // 14: SyncDStar
    {24, 2}, // 15: SyncDStarInv
    {20, 1}, // 16: SyncYSF
    {24, 2}, // 17: SyncP25P1
    {24, 2}, // 18: SyncP25P1Inv
    {24, 2}, // 19: SyncX2TDMADataBS
    {24, 2}, // 20: SyncX2TDMAVoiceBS
    {24, 2}, // 21: SyncX2TDMADataMS
    {24, 2}, // 22: SyncX2TDMAVoiceMS
    {32, 2}, // 23: SyncProVoice
    {32, 2}, // 24: SyncProVoiceInv
    {32, 2}, // 25: SyncProVoiceEA
    {32, 2}, // 26: SyncProVoiceEAInv
};

const unsigned char *DSDSync::getPattern(SyncPattern pattern, int& length)
{
    length = m_syncLenTol[(int) pattern][0];
    return &m_syncPatterns[(int) pattern][m_history - length];
}

void DSDSync::matchAll(const unsigned char *start)
{
    std::fill(m_syncErrors, m_syncErrors + m_patterns, 0);

    for (int i = 0; i < m_history; i++)
    {
        unsigned char c = start[i];

        for (int p = 0; p < m_patterns; p++)
        {
            if (m_syncErrors[p] > m_syncLenTol[p][1]) {
                continue;
            }
            if ((m_syncPatterns[p][i] != 0) && (c != m_syncPatterns[p][i])) {
                m_syncErrors[p]++;
            }
        }
    }
}

void DSDSync::matchSome(const unsigned char *start, int maxHistory, const SyncPattern *patterns, int nbPatterns)
{
    std::fill(m_syncErrors, m_syncErrors + m_patterns, 0);
    int pshift = m_history - maxHistory;

    for (int i = 0; i < maxHistory; i++)
    {
        unsigned char c = start[i];

        for (int ip = 0; ip < nbPatterns; ip++)
        {
            int p = (int) patterns[ip];

            if (m_syncErrors[p] > m_syncLenTol[p][1]) {
                continue;
            }
            if ((m_syncPatterns[p][i+pshift] != 0) && (c != m_syncPatterns[p][i+pshift])) {
                m_syncErrors[p]++;
            }
        }
    }
}

bool DSDSync::isMatching(SyncPattern pattern)
{
    return m_syncErrors[pattern] <= m_syncLenTol[pattern][1];
}

unsigned int DSDSync::getErrors(SyncPattern pattern)
{
    return m_syncErrors[pattern];
}

} // namespace DSDcc
