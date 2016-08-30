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

void decode(DSDcc::QR_16_7_6& qr_16_7_6, unsigned char *codeword)
{
    for (int i = 0; i < 16; i++)
    {
        std::cout << (int) codeword[i] << " ";
    }

    std::cout << std::endl;

    if (qr_16_7_6.decode(codeword))
    {
        for (int i = 0; i < 7; i++)
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
    unsigned char msg[7] = {0, 1, 1, 0, 1, 0, 1};
    unsigned char codeword[16];
    DSDcc::QR_16_7_6 qr_16_7_6;

    qr_16_7_6.encode(msg, codeword);

    std::cout << "No errors" << std::endl;
    decode(qr_16_7_6, codeword);

    std::cout << "Flip one bit (4)" << std::endl;
    codeword[4] ^=  1;
    decode(qr_16_7_6, codeword);

    std::cout << "Flip two bits (1,5)" << std::endl;
    codeword[1] ^= 1;
    codeword[5] ^= 1;
    decode(qr_16_7_6, codeword);

    return 0;
}



