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

#include <iostream>
#include "../fec.h"

void decode(DSDcc::Golay_20_8& Golay_20_8, unsigned char *codeword)
{
    for (int i = 0; i < 20; i++)
    {
        std::cout << (int) codeword[i] << " ";
    }

    std::cout << std::endl;

    if (Golay_20_8.decode(codeword))
    {
        for (int i = 0; i < 8; i++)
        {
            std::cout << (int) codeword[i] << " ";
        }

        std::cout << std::endl << "Decoding OK" << std::endl;
    }
    else
    {
        std::cout << "Decoding error" << std::endl;
    }
}

void rand_test()
{
    unsigned char msg[8];
    unsigned char codeword[20], xcodeword[20];
    int idx1, idx2, idx3;
    int dataIn, dataOut;
    int passCount = 0, failCount = 0, parityFailCount = 0;
    DSDcc::Golay_20_8 golay_20_8;

    // Run multiple times, to randomly corrupt different bits
    // Takes about 10 seconds on a fast PC
    for (int repeat = 0; repeat < 100000; repeat++)
    {
        // Exhaustively test all 8-bit inputs
        for (int dataIn = 0; dataIn < 256; dataIn++)
        {
            // Convert to array of bits
            for (int j = 0; j < 8; j++) {
                msg[j] = (dataIn >> j) & 1;
            }

            // Encode
            golay_20_8.encode(msg, codeword);

            // Save copy of uncorrupted codeword
            std::copy(codeword, codeword + 20, xcodeword);

            // Randomly corrupt up to 3 bits
            idx1 = rand() % 20;
            idx2 = rand() % 20;
            idx3 = rand() % 20;
            codeword[idx1] ^= codeword[idx1];
            codeword[idx2] ^= codeword[idx2];
            codeword[idx3] ^= codeword[idx3];

            bool fail = false;
            // Decode and correct errors
            dataOut = 0;
            if (golay_20_8.decode(codeword))
            {
                // Check data is corrected
                for (int j = 0; j < 8; j++) {
                    dataOut |= codeword[j] << j;
                }
                if (dataIn != dataOut) {
                    fail = true;
                }

                // Check also that parity has been corrected, as we previously had a bug with this
                if (memcmp(codeword, xcodeword, 20)) {
                    parityFailCount++;
                }
            }
            else
            {
                fail = true;
            }
            if (fail)
            {
                std::cout << "Decode failed:"
                    << " dataIn=" << dataIn
                    << " dataOut=" << dataOut
                    << " idx1=" << idx1
                    << " idx2=" << idx2
                    << " idx3=" << idx3
                    << "\n";
                failCount++;
            }
            else
            {
                passCount++;
            }
        }
    }

    std::cout << "rand_test:"
        << " Passcount=" << passCount
        << " Failcount=" << failCount
        << " parityFailCount=" << parityFailCount << "\n";
}

int main(int argc, char *argv[])
{
    unsigned char msg[8]  = {1, 0, 0, 1, 0, 1, 0, 0};
    unsigned char er0[20] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char codeword[20], xcodeword[20];

    DSDcc::Golay_20_8 golay_20_8;
    golay_20_8.encode(msg, codeword);

    std::cout << "No errors" << std::endl;
    decode(golay_20_8, codeword);

    std::cout << std::endl << "Error on one bit (4)" << std::endl;
    decode(golay_20_8, er0);

    std::cout << std::endl << "Flip one bit (4)" << std::endl;
    std::copy(codeword, codeword + 20, xcodeword);
    xcodeword[4] ^=  1;
    decode(golay_20_8, xcodeword);

    std::cout << std::endl << "Flip one bit (15) in parity" << std::endl;
    std::copy(codeword, codeword + 20, xcodeword);
    xcodeword[15] ^=  1;
    decode(golay_20_8, xcodeword);

    std::cout << std::endl << "Flip two bits (1,5)" << std::endl;
    std::copy(codeword, codeword + 20, xcodeword);
    xcodeword[1] ^= 1;
    xcodeword[5] ^= 1;
    decode(golay_20_8, xcodeword);

    std::cout << std::endl << "Flip two bits (1,15) - one in parity" << std::endl;
    std::copy(codeword, codeword + 20, xcodeword);
    xcodeword[1] ^= 1;
    xcodeword[15] ^= 1;
    decode(golay_20_8, xcodeword);

    std::cout << std::endl << "Flip three bits (1,5,6)" << std::endl;
    std::copy(codeword, codeword + 20, xcodeword);
    xcodeword[1] ^= 1;
    xcodeword[5] ^= 1;
    xcodeword[6] ^= 1;
    decode(golay_20_8, xcodeword);

    std::cout << std::endl << "Flip three bits (1,5,9) - one in parity" << std::endl;
    std::copy(codeword, codeword + 20, xcodeword);
    xcodeword[1] ^= 1;
    xcodeword[5] ^= 1;
    xcodeword[9] ^= 1;
    decode(golay_20_8, xcodeword);

    std::cout << std::endl << "Flip three bits (1,15,21) - 2 in parity" << std::endl;
    std::copy(codeword, codeword + 20, xcodeword);
    xcodeword[1] ^= 1;
    xcodeword[15] ^= 1;
    xcodeword[19] ^= 1;
    decode(golay_20_8, xcodeword);

    std::cout << std::endl << "Flip three bits (15,18,21) - all parity" << std::endl;
    std::copy(codeword, codeword + 20, xcodeword);
    xcodeword[15] ^= 1;
    xcodeword[18] ^= 1;
    xcodeword[19] ^= 1;
    decode(golay_20_8, xcodeword);

    rand_test();

    return 0;
}



