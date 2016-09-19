///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 Edouard Griffiths, F4EXB.                                  //
//                                                                               //
// Thanks to Sven Reifegerste: http://www.zorc.breitbandkatze.de/crc.html        //
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
#include "../crc.h"

void testYSF(DSDcc::CRC& crc, const unsigned char *bytes, const char *comment, const unsigned int crcCorrect)
{
    unsigned long ret_crcbitbybit     = crc.crcbitbybit((unsigned char *)bytes, 4);
    unsigned long ret_crcbitbybitfast = crc.crcbitbybitfast((unsigned char *)bytes, 4);
    unsigned long ret_crctable        = crc.crctable((unsigned char *)bytes, 4);
    unsigned long ret_crctablefast    = crc.crctablefast((unsigned char *)bytes, 4);

    std::cout << std::endl << "CCITT16 " << comment << " :" << std::endl;
    std::cout << "crc bit by bit      :  " << std::hex << ret_crcbitbybit << (ret_crcbitbybit == crcCorrect ? " OK" : " KO")<< std::endl;
    std::cout << "crc bit by bit fast :  " << std::hex << ret_crcbitbybitfast << (ret_crcbitbybitfast == crcCorrect ? " OK" : " KO") << std::endl;
    std::cout << "crc table           :  " << std::hex << ret_crctable << (ret_crctable == crcCorrect ? " OK" : " KO") << std::endl;
    std::cout << "crc table fast      :  " << std::hex << ret_crctablefast << (ret_crctablefast == crcCorrect ? " OK" : " KO") << std::endl;
}

int main(int argc, char *argv[])
{
    DSDcc::CRC crc(DSDcc::CRC::PolyCCITT16, 16, 0xffff, 0xffff, 1, 1, 1);
    const char string[] = {"1234"};
    const unsigned int crcCorrect_ccitt16_defaults = 0x74ec;

    unsigned long ret_crcbitbybit     = crc.crcbitbybit((unsigned char *)string, (unsigned long) strlen(string));
    unsigned long ret_crcbitbybitfast = crc.crcbitbybitfast((unsigned char *)string, (unsigned long) strlen(string));
    unsigned long ret_crctable        = crc.crctable((unsigned char *)string, (unsigned long) strlen(string));
    unsigned long ret_crctablefast    = crc.crctablefast((unsigned char *)string, (unsigned long) strlen(string));

    std::cout << "CCITT16 with defaults:" << std::endl;
    std::cout << "crc bit by bit      :  " << std::hex << ret_crcbitbybit << (ret_crcbitbybit == crcCorrect_ccitt16_defaults ? " OK" : " *KO*") << std::endl;
    std::cout << "crc bit by bit fast :  " << std::hex << ret_crcbitbybitfast << (ret_crcbitbybitfast == crcCorrect_ccitt16_defaults ? " OK" : " *KO*") << std::endl;
    std::cout << "crc table           :  " << std::hex << ret_crctable << (ret_crctable == crcCorrect_ccitt16_defaults ? " OK" : " *KO*") << std::endl;
    std::cout << "crc table fast      :  " << std::hex << ret_crctablefast << (ret_crctablefast == crcCorrect_ccitt16_defaults ? " OK" : " *KO*") << std::endl;

    const unsigned char bytes01[] = {0x60, 0x36, 0x02, 0x00};
    const unsigned char bytes02[] = {0x60, 0x16, 0x02, 0x00};
    const unsigned char bytes03[] = {0x87, 0x16, 0x02, 0x00};

    DSDcc::CRC crc000(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff, 0, 0, 0);
    DSDcc::CRC crc001(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff, 0, 0, 1);
    DSDcc::CRC crc010(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff, 0, 1, 0);
    DSDcc::CRC crc011(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff, 0, 1, 1);

    DSDcc::CRC crc100(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff, 1, 0, 0);
    DSDcc::CRC crc101(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff, 1, 0, 1);
    DSDcc::CRC crc110(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff, 1, 1, 0);
    DSDcc::CRC crc111(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff, 1, 1, 1);

    std::cout << std::endl << "All cases for YSF" << std::endl;

    testYSF(crc000, bytes01, "0x60, 0x36, 0x02, 0x00 000", 0xb74a);
    testYSF(crc001, bytes01, "0x60, 0x36, 0x02, 0x00 001", 0x52ed);
    testYSF(crc010, bytes01, "0x60, 0x36, 0x02, 0x00 010", 0x3ba0);
    testYSF(crc011, bytes01, "0x60, 0x36, 0x02, 0x00 011", 0x5dc);

    testYSF(crc100, bytes01, "0x60, 0x36, 0x02, 0x00 100", 0xb74a);
    testYSF(crc101, bytes01, "0x60, 0x36, 0x02, 0x00 101", 0x52ed);
    testYSF(crc110, bytes01, "0x60, 0x36, 0x02, 0x00 110", 0x3ba0);
    testYSF(crc111, bytes01, "0x60, 0x36, 0x02, 0x00 111", 0x5dc);

    std::cout << std::endl << "Final for YSF" << std::endl;

    DSDcc::CRC crcYSF(DSDcc::CRC::PolyCCITT16, 16, 0x0, 0xffff); // default flags

    testYSF(crcYSF, bytes01, "0x60, 0x36, 0x02, 0x00 100", 0xb74a);
    testYSF(crcYSF, bytes02, "0x60, 0x16, 0x02, 0x00 100", 0x318c);
    testYSF(crcYSF, bytes03, "0x87, 0x36, 0x02, 0x00 100", 0xe44b);

    return 0;
}
