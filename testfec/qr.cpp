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
#include <string.h>
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

        std::cout << std::endl << "Decoding OK" << std::endl;
    }
    else
    {
        std::cout << "Decoding error" << std::endl;
    }
}

void listAll(DSDcc::QR_16_7_6& qr_16_7_6, unsigned int *codewordValues)
{
	unsigned char msg[7];
	unsigned char codeword[16];
	int i = 0;

	for (unsigned char b6 = 0; b6 < 2; b6++)
	{
		msg[0] = b6;
		for (unsigned char b5 = 0; b5 < 2; b5++)
		{
			msg[1] = b5;
			for (unsigned char b4 = 0; b4 < 2; b4++)
			{
				msg[2] = b4;
				for (unsigned char b3 = 0; b3 < 2; b3++)
				{
					msg[3] = b3;
					for (unsigned char b2 = 0; b2 < 2; b2++)
					{
						msg[4] = b2;
						for (unsigned char b1 = 0; b1 < 2; b1++)
						{
							msg[5] = b1;
							for (unsigned char b0 = 0; b0 < 2; b0++)
							{
								msg[6] = b0;
								qr_16_7_6.encode(msg, codeword);

								codewordValues[i] = (codeword[15])
										+ (codeword[14]<<1)
										+ (codeword[13]<<2)
										+ (codeword[12]<<3)
										+ (codeword[11]<<4)
										+ (codeword[10]<<5)
										+ (codeword[9]<<6)
										+ (codeword[8]<<7)
										+ (codeword[7]<<8)
										+ (codeword[6]<<9)
										+ (codeword[5]<<10)
										+ (codeword[4]<<11)
										+ (codeword[3]<<12)
										+ (codeword[2]<<13)
										+ (codeword[1]<<14)
										+ (codeword[0]<<15);
								i++;
							}
						}
					}
				}
			}
		}
	}
}


int main(int argc, char *argv[])
{
    unsigned char msg[7] = {0, 1, 1, 0, 1, 0, 1};
    unsigned char codeword[16], xcodeword[16];
    DSDcc::QR_16_7_6 qr_16_7_6;
    static const unsigned int validCodewordValues[128] = {
    		0,627,1253,1686,2505,3002,3372,3935,4578,5009,5383,6004,6187,
			6744,7374,7869,8631,9156,9554,10017,10366,10765,11419,12008,12373,12838,13488,
			14019,14748,15343,15737,16138,16670,17261,17915,18312,18647,19108,19506,20033,
			20732,21135,21529,22122,22837,23366,24016,24483,24745,25306,25676,26175,26976,
			27411,28037,28662,29003,29496,30126,30685,30850,31473,31847,32276,32847,33340,
			33962,34521,35206,35829,36195,36624,37293,37854,38216,38715,39012,39447,40065,
			40690,41464,41867,42269,42862,43057,43586,44244,44711,45082,45673,46335,46732,
			47571,48032,48438,48965,49489,49954,50612,51143,51352,51947,52349,52750,53427,
			53952,54358,54821,55674,56073,56735,57324,57574,58005,58371,58992,59695,60252,
			60874,61369,61700,62327,62945,63378,63693,64190,64552,65115
    };

    qr_16_7_6.encode(msg, codeword);

    std::cout << "No errors" << std::endl;
    decode(qr_16_7_6, codeword);

    std::cout << "Flip one bit (4)" << std::endl;
	std::copy(codeword, codeword + 16, xcodeword);
    xcodeword[4] ^= 1;
    decode(qr_16_7_6, xcodeword);

	for (int i = 0; i < 9; i++)
	{
		std::cout << "Flip one parity bit " << 7+i << std::endl;
		std::copy(codeword, codeword + 16, xcodeword);
		xcodeword[7+i] ^= 1;
		decode(qr_16_7_6, xcodeword);
	}

    std::cout << "Flip two bits (1,5)" << std::endl;
	std::copy(codeword, codeword + 16, xcodeword);
    xcodeword[1] ^= 1;
    xcodeword[5] ^= 1;
    decode(qr_16_7_6, xcodeword);

	for (int i = 0; i < 9; i++)
	{
		std::cout << "Flip two bits (1) and one parity bit " << 7+i << std::endl;
		std::copy(codeword, codeword + 16, xcodeword);
	    xcodeword[1] ^= 1;
		xcodeword[7+i] ^= 1;
		decode(qr_16_7_6, xcodeword);
	}

    std::cout << "Verify valid codewords values" << std::endl;
    unsigned int calculatedValidCodewordValues[128];
    listAll(qr_16_7_6, calculatedValidCodewordValues);

    if (memcmp(calculatedValidCodewordValues, validCodewordValues, 128) == 0) {
    	std::cout << "valid codewords OK" << std::endl;
    } else {
    	std::cout << "valid codewords KO" << std::endl;
    }

    return 0;
}



