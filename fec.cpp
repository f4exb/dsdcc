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
#include "fec.h"

namespace DSDcc
{

const unsigned char Hamming_7_4::m_G[7*4] = {
        1, 0, 0, 0,   1, 0, 1,
        0, 1, 0, 0,   1, 1, 1,
        0, 0, 1, 0,   1, 1, 0,
        0, 0, 0, 1,   0, 1, 1,
};

const unsigned char Hamming_7_4::m_H[7*3] = {
        1, 1, 1, 0,   1, 0, 0,
        0, 1, 1, 1,   0, 1, 0,
        1, 1, 0, 1,   0, 0, 1
//      0  1  2  3 <- correctable bit positions
};

void Hamming_7_4::init()
{
    // correctable bit positions given syndrome bits as index (see above)
    memset(m_corr, 0xFF, 8); // initialize with all invalid positions
    m_corr[0b101] = 0;
    m_corr[0b111] = 1;
    m_corr[0b110] = 2;
    m_corr[0b011] = 3;
}

// ========================================================================================

const unsigned char Hamming_12_8::m_G[12*8] = {
        1, 0, 0, 0, 0, 0, 0, 0,   1, 1, 1, 0,
        0, 1, 0, 0, 0, 0, 0, 0,   0, 1, 1, 1,
        0, 0, 1, 0, 0, 0, 0, 0,   1, 0, 1, 0,
        0, 0, 0, 1, 0, 0, 0, 0,   0, 1, 0, 1,
        0, 0, 0, 0, 1, 0, 0, 0,   1, 0, 1, 1,
        0, 0, 0, 0, 0, 1, 0, 0,   1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0,   0, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 1,   0, 0, 1, 1,
};

const unsigned char Hamming_12_8::m_H[12*4] = {
        1, 0, 1, 0, 1, 1, 0, 0,   1, 0, 0, 0,
        1, 1, 0, 1, 0, 1, 1, 0,   0, 1, 0, 0,
        1, 1, 1, 0, 1, 0, 1, 1,   0, 0, 1, 0,
        0, 1, 0, 1, 1, 0, 0, 1,   0, 0, 0, 1
//      0  1  2  3  4  5  6  7 <- correctable bit positions
};

void Hamming_12_8::init()
{
    // correctable bit positions given syndrome bits as index (see above)
    memset(m_corr, 0xFF, 16); // initialize with all invalid positions
    m_corr[0b1110] = 0;
    m_corr[0b0111] = 1;
    m_corr[0b1010] = 2;
    m_corr[0b0101] = 3;
    m_corr[0b1011] = 4;
    m_corr[0b1100] = 5;
    m_corr[0b0110] = 6;
    m_corr[0b0011] = 7;
}

// ========================================================================================

const unsigned char Golay_20_8::m_G[20*8] = {
        1, 0, 0, 0, 0, 0, 0, 0,    0, 0, 1, 1,  1, 1, 0, 1,  1, 0, 1, 0,
        0, 1, 0, 0, 0, 0, 0, 0,    1, 1, 0, 1,  1, 0, 0, 1,  1, 0, 0, 1,
        0, 0, 1, 0, 0, 0, 0, 0,    0, 1, 1, 0,  1, 1, 0, 0,  1, 1, 0, 1,
        0, 0, 0, 1, 0, 0, 0, 0,    0, 0, 1, 1,  0, 1, 1, 0,  0, 1, 1, 1,
        0, 0, 0, 0, 1, 0, 0, 0,    1, 1, 0, 1,  1, 1, 0, 0,  0, 1, 1, 0,
        0, 0, 0, 0, 0, 1, 0, 0,    1, 0, 1, 0,  1, 0, 0, 1,  0, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 1, 0,    1, 0, 0, 1,  0, 0, 1, 1,  1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 1,    1, 0, 0, 0,  1, 1, 1, 0,  1, 0, 1, 1,
};

const unsigned char Golay_20_8::m_H[20*12] = {
        0, 1, 0, 0, 1, 1, 1, 1,    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 0, 1, 0, 0, 0,    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 1, 1, 0, 1, 0, 0,    0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 0, 1, 1, 0, 1, 0,    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 0, 1, 1, 0, 1,    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 1, 1, 1, 0, 0, 1,    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 1, 1,    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
        1, 1, 0, 0, 0, 1, 1, 0,    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        1, 1, 1, 0, 0, 0, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
        1, 0, 0, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
        0, 1, 1, 1, 0, 1, 0, 1,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
};

// ========================================================================================

const unsigned char QR_16_7_6::m_G[16*7] = {
        1, 0, 0, 0, 0, 0, 0,    0, 0, 1, 0, 0, 1, 1, 1, 1,
        0, 1, 0, 0, 0, 0, 0,    1, 0, 0, 0, 1, 1, 1, 1, 0,
        0, 0, 1, 0, 0, 0, 0,    1, 1, 0, 1, 1, 0, 1, 1, 1,
        0, 0, 0, 1, 0, 0, 0,    1, 1, 1, 1, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 1, 0, 0,    1, 1, 1, 0, 0, 1, 0, 0, 1,
        0, 0, 0, 0, 0, 1, 0,    0, 1, 1, 1, 0, 0, 1, 0, 1,
        0, 0, 0, 0, 0, 0, 1,    0, 0, 1, 1, 1, 0, 0, 1, 1,
};
const unsigned char QR_16_7_6::m_H[16*9] = {
        0, 1, 1,  1, 1, 0, 0,   1, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1,  1, 1, 1, 0,   0, 1, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0,  1, 1, 1, 1,   0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 1,  1, 0, 1, 1,   0, 0, 0, 1, 0, 0, 0, 0, 0,
        0, 1, 1,  0, 0, 0, 1,   0, 0, 0, 0, 1, 0, 0, 0, 0,
        1, 1, 0,  0, 1, 0, 0,   0, 0, 0, 0, 0, 1, 0, 0, 0,
        1, 1, 1,  0, 0, 1, 0,   0, 0, 0, 0, 0, 0, 1, 0, 0,
        1, 1, 1,  1, 0, 0, 1,   0, 0, 0, 0, 0, 0, 0, 1, 0,
        1, 0, 1,  0, 1, 1, 1,   0, 0, 0, 0, 0, 0, 0, 0, 1,
};

// ========================================================================================

Hamming_7_4::Hamming_7_4()
{
    init();
}

Hamming_7_4::~Hamming_7_4()
{
}

// Not very efficient but encode is used for unit testing only
void Hamming_7_4::encode(unsigned char *origBits, unsigned char *encodedBits)
{
    memset(encodedBits, 0, 7);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            encodedBits[j] += origBits[i] * m_G[7*i + j];
        }
    }

    for (int i = 0; i < 7; i++)
    {
        encodedBits[i] %= 2;
    }
}

bool Hamming_7_4::decode(unsigned char *rxBits) // corrects in place
{
    unsigned int syndromeI = 0; // syndrome index

    for (int is = 0; is < 3; is++)
    {
        syndromeI += (((rxBits[0] * m_H[7*is + 0])
                    + (rxBits[1] * m_H[7*is + 1])
                    + (rxBits[2] * m_H[7*is + 2])
                    + (rxBits[3] * m_H[7*is + 3])
                    + (rxBits[4] * m_H[7*is + 4])
                    + (rxBits[5] * m_H[7*is + 5])
                    + (rxBits[6] * m_H[7*is + 6])) % 2) << (2-is);
    }

    if (syndromeI > 0)
    {
        if (m_corr[syndromeI] == 0xFF)
        {
            return false;
        }
        else
        {
            rxBits[m_corr[syndromeI]] ^= 1; // flip bit
        }
    }

    return true;
}

// ========================================================================================

Hamming_12_8::Hamming_12_8()
{
    init();
}

Hamming_12_8::~Hamming_12_8()
{
}

// Not very efficient but encode is used for unit testing only
void Hamming_12_8::encode(unsigned char *origBits, unsigned char *encodedBits)
{
    memset(encodedBits, 0, 12);

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            encodedBits[j] += origBits[i] * m_G[12*i + j];
        }
    }

    for (int i = 0; i < 12; i++)
    {
        encodedBits[i] %= 2;
    }
}

bool Hamming_12_8::decode(unsigned char *rxBits, unsigned char *decodedBits, int nbCodewords)
{
    bool correctable = true;

    for (int ic = 0; ic < nbCodewords; ic++)
    {
        // calculate syndrome

        bool error = false;
        int syndromeI = 0; // syndrome index

        for (int is = 0; is < 4; is++)
        {
            syndromeI += (((rxBits[12*ic +  0] * m_H[12*is +  0])
                         + (rxBits[12*ic +  1] * m_H[12*is +  1])
                         + (rxBits[12*ic +  2] * m_H[12*is +  2])
                         + (rxBits[12*ic +  3] * m_H[12*is +  3])
                         + (rxBits[12*ic +  4] * m_H[12*is +  4])
                         + (rxBits[12*ic +  5] * m_H[12*is +  5])
                         + (rxBits[12*ic +  6] * m_H[12*is +  6])
                         + (rxBits[12*ic +  7] * m_H[12*is +  7])
                         + (rxBits[12*ic +  8] * m_H[12*is +  8])
                         + (rxBits[12*ic +  9] * m_H[12*is +  9])
                         + (rxBits[12*ic + 10] * m_H[12*is + 10])
                         + (rxBits[12*ic + 11] * m_H[12*is + 11])) % 2) << (3-is);
        }

        // correct bit

        if (syndromeI > 0) // single bit error correction
        {
            if (m_corr[syndromeI] == 0xFF) // uncorrectable error
            {
                correctable = false;
            }
            else
            {
                rxBits[m_corr[syndromeI]] ^= 1; // flip bit
            }
        }

        // move information bits
        memcpy(&decodedBits[8*ic], &rxBits[12*ic], 8);
    }

    return correctable;
}

// ========================================================================================

Golay_20_8::Golay_20_8()
{
    init();
}

Golay_20_8::~Golay_20_8()
{
}

void Golay_20_8::init()
{
    memset (m_corr, 0xFF, 3*4096);

    for (int i1 = 0; i1 < 8; i1++)
    {
        for (int i2 = i1+1; i2 < 8; i2++)
        {
            for (int i3 = i2+1; i3 < 8; i3++)
            {
                // 3 bit patterns
                int syndromeI = 0;

                for (int ir = 0; ir < 12; ir++)
                {
                    syndromeI += ((m_H[20*ir + i1] +  m_H[20*ir + i2] +  m_H[20*ir + i3]) % 2) << (11-ir);
                }

                m_corr[syndromeI][0] = i1;
                m_corr[syndromeI][1] = i2;
                m_corr[syndromeI][2] = i3;
            }

            // 2 bit patterns
            int syndromeI = 0;

            for (int ir = 0; ir < 12; ir++)
            {
                syndromeI += ((m_H[20*ir + i1] +  m_H[20*ir + i2]) % 2) << (11-ir);
            }

            m_corr[syndromeI][0] = i1;
            m_corr[syndromeI][1] = i2;
        }

        // single bit patterns
        int syndromeI = 0;

        for (int ir = 0; ir < 12; ir++)
        {
            syndromeI += m_H[20*ir + i1] << (11-ir);
        }

        m_corr[syndromeI][0] = i1;
    }
}

// Not very efficient but encode is used for unit testing only
void Golay_20_8::encode(unsigned char *origBits, unsigned char *encodedBits)
{
    memset(encodedBits, 0, 20);

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            encodedBits[j] += origBits[i] * m_G[20*i + j];
        }
    }

    for (int i = 0; i < 20; i++)
    {
        encodedBits[i] %= 2;
    }
}

bool Golay_20_8::decode(unsigned char *rxBits)
{
    unsigned int syndromeI = 0; // syndrome index

    for (int is = 0; is < 12; is++)
    {
        syndromeI += (((rxBits[0] * m_H[20*is + 0])
                    + (rxBits[1] * m_H[20*is + 1])
                    + (rxBits[2] * m_H[20*is + 2])
                    + (rxBits[3] * m_H[20*is + 3])
                    + (rxBits[4] * m_H[20*is + 4])
                    + (rxBits[5] * m_H[20*is + 5])
                    + (rxBits[6] * m_H[20*is + 6])
                    + (rxBits[7] * m_H[20*is + 7])
                    + (rxBits[8] * m_H[20*is + 8])
                    + (rxBits[9] * m_H[20*is + 9])
                    + (rxBits[10] * m_H[20*is + 10])
                    + (rxBits[11] * m_H[20*is + 11])
                    + (rxBits[12] * m_H[20*is + 12])
                    + (rxBits[13] * m_H[20*is + 13])
                    + (rxBits[14] * m_H[20*is + 14])
                    + (rxBits[15] * m_H[20*is + 15])
                    + (rxBits[16] * m_H[20*is + 16])
                    + (rxBits[17] * m_H[20*is + 17])
                    + (rxBits[18] * m_H[20*is + 18])
                    + (rxBits[19] * m_H[20*is + 19])) % 2) << (11-is);
    }

    if (syndromeI > 0)
    {
        int i = 0;

        for (; i < 3; i++)
        {
            if (m_corr[syndromeI][i] == 0xFF)
            {
                break;
            }
            else
            {
                rxBits[m_corr[syndromeI][i]] ^= 1; // flip bit
            }
        }

        if (i == 0)
        {
            return false;
        }
    }

    return true;
}

// ========================================================================================

QR_16_7_6::QR_16_7_6()
{
    init();
}

QR_16_7_6::~QR_16_7_6()
{
}

void QR_16_7_6::init()
{
    memset (m_corr, 0xFF, 2*512);

    for (int i1 = 0; i1 < 7; i1++)
    {
        for (int i2 = i1+1; i2 < 7; i2++)
        {
            // 2 bit patterns
            int syndromeI = 0;

            for (int ir = 0; ir < 9; ir++)
            {
                syndromeI += ((m_H[16*ir + i1] +  m_H[16*ir + i2]) % 2) << (8-ir);
            }

            m_corr[syndromeI][0] = i1;
            m_corr[syndromeI][1] = i2;
        }

        // single bit patterns
        int syndromeI = 0;

        for (int ir = 0; ir < 9; ir++)
        {
            syndromeI += m_H[16*ir + i1] << (8-ir);
        }

        m_corr[syndromeI][0] = i1;
    }
}

// Not very efficient but encode is used for unit testing only
void QR_16_7_6::encode(unsigned char *origBits, unsigned char *encodedBits)
{
    memset(encodedBits, 0, 16);

    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            encodedBits[j] += origBits[i] * m_G[16*i + j];
        }
    }

    for (int i = 0; i < 16; i++)
    {
        encodedBits[i] %= 2;
    }
}

bool QR_16_7_6::decode(unsigned char *rxBits)
{
    unsigned int syndromeI = 0; // syndrome index

    for (int is = 0; is < 9; is++)
    {
        syndromeI += (((rxBits[0] * m_H[16*is + 0])
                    + (rxBits[1] * m_H[16*is + 1])
                    + (rxBits[2] * m_H[16*is + 2])
                    + (rxBits[3] * m_H[16*is + 3])
                    + (rxBits[4] * m_H[16*is + 4])
                    + (rxBits[5] * m_H[16*is + 5])
                    + (rxBits[6] * m_H[16*is + 6])
                    + (rxBits[7] * m_H[16*is + 7])
                    + (rxBits[8] * m_H[16*is + 8])
                    + (rxBits[9] * m_H[16*is + 9])
                    + (rxBits[10] * m_H[16*is + 10])
                    + (rxBits[11] * m_H[16*is + 11])
                    + (rxBits[12] * m_H[16*is + 12])
                    + (rxBits[13] * m_H[16*is + 13])
                    + (rxBits[14] * m_H[16*is + 14])
                    + (rxBits[15] * m_H[16*is + 15])) % 2) << (8-is);
    }

    if (syndromeI > 0)
    {
        int i = 0;

        for (; i < 2; i++)
        {
            if (m_corr[syndromeI][i] == 0xFF)
            {
                break;
            }
            else
            {
                rxBits[m_corr[syndromeI][i]] ^= 1; // flip bit
            }
        }

        if (i == 0)
        {
            return false;
        }
    }

    return true;
}

} // namespace DSDcc
