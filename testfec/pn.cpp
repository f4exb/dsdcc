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
#include <iomanip>
#include <string.h>
#include "../pn.h"

unsigned char NXDN_PN_REF[48] = {0x27, 0x2a, 0xc3, 0x7a, 0x6e, 0x45, 0xa, 0xd3, 0xf6, 0x49, 0x6f, 0xc9, 0xa9, 0x98, 0xc, 0x65, 0x1a, 0x5f, 0xd1, 0x63, 0xac, 0xb3, 0xc7, 0xdd, 0x6, 0xb6, 0xec, 0x16, 0xbe, 0xaa, 0x5, 0x2b, 0xcb, 0xb8, 0x1c, 0xe9, 0x3d, 0x75, 0x12, 0x19, 0xc2, 0xf6, 0xcd, 0xe, 0xf0, 0xff, 0x83, 0xdf};


void nxdn_scrambler_test()
{
    DSDcc::PN_9_5 pn(0xe4);

    //std::cerr << "NXDN scrambler test" << std::endl;

    for (int i = 0; i < 48; i++) {
        //std::cerr << "0x" << std::hex << (int) pn.getByte(i) << ", ";
        unsigned char bx = pn.getByte(i);
        if (bx != NXDN_PN_REF[i])
        {
            std::cerr << "NXDN scrambler test failed at index " << i << ": " << std::hex << (int) bx << " != " << (int) NXDN_PN_REF[i] <<  std::endl;
            return;
        }
    }
    //std::cerr << std::endl;

    std::cerr << "NXDN scrambler test OK" << std::endl;
}


int main(int argc, char *argv[])
{
    const unsigned char validSequence[] = {1,0,0,1,0,0,1,1,1,1,0,1,0,1,1,1,0,1,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,0,1,1,1,0,0,0,0,1,0,1,1,1,1,0,1,1,0,1,1,0,0,1,1,0,1,0,0,0,0,1,1,1,0,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,1,1,1,1,1,0,0,0,1,0,1,1,1,0,0,1,1,0,0,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,1,1,0,1,1,0,1,0,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    DSDcc::PN_9_5 pn(0x1c9);

    for (int i = 0; i < 16; i++)
    {
        std::cout << (int) pn.getBit(i) << " ";
    }

    std::cout << std::endl;

    for (int i = 0; i < 16; i++)
    {
        std::cout << (int) validSequence[i] << " ";
    }

    std::cout << std::endl;

    if (memcmp(pn.getBits(), validSequence, 160) == 0)
    {
        std::cerr << "160 first bits OK" << std::endl;
    }
    else
    {
        std::cerr << "Sequence KO" << std::endl;
    }

    nxdn_scrambler_test();
}



