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

void decode(DSDcc::Hamming_12_8& hamming_12_8, unsigned char *codeword)
{
    unsigned char decoded[8];

    for (int i = 0; i < 12; i++)
    {
        std::cout << (int) codeword[i] << " ";
    }

    std::cout << std::endl;

    if (hamming_12_8.decode(codeword, decoded, 1))
    {
        for (int i = 0; i < 8; i++)
        {
            std::cout << (int) decoded[i] << " ";
        }

        std::cout << std::endl << "Decoding OK" << std::endl;
    }
    else
    {
        std::cout << "Decoding error" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    unsigned char msg[8]  = {1, 0, 0, 1, 1, 1, 0, 1};
    unsigned char er0[12] = {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char codeword[12], xcodeword[12];

    DSDcc::Hamming_12_8 hamming_12_8;
    hamming_12_8.encode(msg, codeword);

    std::cout << "No errors" << std::endl;
    decode(hamming_12_8, codeword);

    std::cout << std::endl << "Error (2)" << std::endl;
    decode(hamming_12_8, er0);

    std::cout << std::endl << "Flip one bit (2)" << std::endl;
    std::copy(codeword, codeword + 12, xcodeword);
    xcodeword[2] ^=  1;
    decode(hamming_12_8, xcodeword);

    for (int i = 0; i < 4; i++)
    {
        std::cout << std::endl << "Flip one parity bit " << 8+i << std::endl;
        std::copy(codeword, codeword + 12, xcodeword);
        xcodeword[8+i] ^=  1;
        decode(hamming_12_8, xcodeword);
    }

    return 0;
}




