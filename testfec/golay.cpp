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

        std::cout << std::endl;
    }
    else
    {
        std::cout << "Decoding error" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    unsigned char msg[8]  = {1, 0, 0, 1, 0, 1, 0, 0};
    unsigned char er0[20] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char codeword[20];

    DSDcc::Golay_20_8 golay_20_8;
    golay_20_8.encode(msg, codeword);

    std::cout << "No errors" << std::endl;
    decode(golay_20_8, codeword);

    std::cout << std::endl << "Error on one bit (4)" << std::endl;
    decode(golay_20_8, er0);

    std::cout << std::endl << "Flip one bit (4)" << std::endl;
    codeword[4] ^=  1;
    decode(golay_20_8, codeword);

    std::cout << std::endl << "Flip two bits (1,5)" << std::endl;
    codeword[1] ^= 1;
    codeword[5] ^= 1;
    decode(golay_20_8, codeword);

    std::cout << std::endl << "Flip three bits (1,5,6)" << std::endl;
    codeword[1] ^= 1;
    codeword[5] ^= 1;
    codeword[6] ^= 1;
    decode(golay_20_8, codeword);

    return 0;
}



