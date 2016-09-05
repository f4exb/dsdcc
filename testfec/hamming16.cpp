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

void decode(DSDcc::Hamming_16_11_4& hamming_16_11_4, unsigned char *codeword)
{
    unsigned char decoded[11];

    for (int i = 0; i < 16; i++)
    {
        std::cout << (int) codeword[i] << " ";
    }

    std::cout << std::endl;

    if (hamming_16_11_4.decode(codeword, decoded, 1))
    {
        for (int i = 0; i < 11; i++)
        {
            std::cout << (int) decoded[i] << " ";
        }

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Decoding error" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    unsigned char msg[11]  = {1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1};
    unsigned char er0[16] = {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char codeword[16];

    DSDcc::Hamming_16_11_4 hamming_16_11_4;
    hamming_16_11_4.encode(msg, codeword);

    std::cout << "No errors" << std::endl;
    decode(hamming_16_11_4, codeword);

    std::cout << std::endl << "Error (2)" << std::endl;
    decode(hamming_16_11_4, er0);

    std::cout << std::endl << "Flip one bit (2)" << std::endl;
    codeword[2] ^=  1;
    decode(hamming_16_11_4, codeword);

    std::cout << std::endl << "Flip two bits (2,5)" << std::endl;
    codeword[2] ^=  1;
    codeword[5] ^=  1;
    decode(hamming_16_11_4, codeword);

    return 0;
}




