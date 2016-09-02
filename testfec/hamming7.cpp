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

void decode(DSDcc::Hamming_7_4& hamming_7_4, unsigned char *codeword)
{
    for (int i = 0; i < 7; i++)
    {
        std::cout << (int) codeword[i] << " ";
    }

    std::cout << std::endl;

    if (hamming_7_4.decode(codeword))
    {
        for (int i = 0; i < 4; i++)
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
    unsigned char msg[4]  = {1, 0, 0, 1};
    unsigned char er0[7] = {0, 0, 1, 0, 0, 0, 0};
    unsigned char codeword[7];

    DSDcc::Hamming_7_4 hamming_7_4;
    hamming_7_4.encode(msg, codeword);

    std::cout << "No errors" << std::endl;
    decode(hamming_7_4, codeword);

    std::cout << std::endl << "Error (2)" << std::endl;
    decode(hamming_7_4, er0);

    std::cout << std::endl << "Flip one bit (2)" << std::endl;
    codeword[2] ^=  1;
    decode(hamming_7_4, codeword);

    std::cout << std::endl << "Valid checksums" << std::endl;

    for (int i = 0; i < 16; i++)
    {
    	msg[0] = (i>>3) & 1;
    	msg[1] = (i>>2) & 1;
    	msg[2] = (i>>1) & 1;
    	msg[3] = i & 1;

    	hamming_7_4.encode(msg, codeword);
    	int codenumber = 0;

    	for (int j = 0; j < 7; j++)
    	{
    		codenumber += codeword[j] * (1<<(6-j));
    	}

    	std::cout << codenumber << std::endl;
    }

    return 0;
}




