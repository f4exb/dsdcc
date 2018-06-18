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
#include "../nxdncrc.h"

const uint8_t  BIT_MASK_TABLE1[] = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE1[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE1[(i)&7])
#define READ_BIT1(p,i)    (p[(i)>>3] & BIT_MASK_TABLE1[(i)&7])

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

void testNXDN()
{
    unsigned char test1[] = {0x60, 0x36, 0x02, 0x00};
    uint16_t test_crc1 = DSDcc::CNXDNCRC::createCRC16(test1, 32U);
    std::cout << "NXDN CCITT16 for test (1): " << std::hex << test_crc1 << std::endl;
    unsigned char test2[] = {0x01, 0x19, 0x40, 0x7f, 0x61, 0xb3, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x83, 0x00};
    uint16_t test_crc2 = DSDcc::CNXDNCRC::createCRC16(test2, 155U);
    std::cout << "NXDN CCITT16 for test (2): " << std::hex << test_crc2 << std::endl;
    uint8_t temp[2];
    for (int i = 0, j = 155; i < 16; i++, j++)
    {
        bool b = READ_BIT1(test2, j);
        WRITE_BIT1(temp, i, b);
    }
    std::cout << "NXDN CCITT16 for test (2): " << std::hex << (unsigned int) temp[0] << ":" << (unsigned int) temp[1] << std::endl;
    bool test = DSDcc::CNXDNCRC::checkCRC16(test2, 155U);
    std::cout << "NXDN CCITT16 for test (2): " << test << std::endl;
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

    unsigned char dstarHeader[41] = {
    		0x40, 0x00, 0x00, 0x44, 0x42,
			0x30, 0x44, 0x46, 0x20, 0x20,
			0x42, 0x44, 0x42, 0x30, 0x44,
			0x46, 0x20, 0x20, 0x42, 0x43,
			0x51, 0x43, 0x51, 0x43, 0x51,
			0x20, 0x20, 0x44, 0x4f, 0x36,
			0x54, 0x4f, 0x42, 0x20, 0x20,
			0x20, 0x20, 0x20, 0x20, 0x45,
			0x26
    };

    unsigned char dstarCRCGPS[71+2] = {
    		0x44, 0x4c, 0x33, 0x4f, 0x43,
			0x4b, 0x3e, 0x41, 0x50, 0x49,
			0x32, 0x38, 0x32, 0x2c, 0x44,
			0x53, 0x54, 0x41, 0x52, 0x2a,
			0x3a, 0x2f, 0x32, 0x31, 0x31,
			0x32, 0x33, 0x34, 0x68, 0x35,
			0x32, 0x33, 0x30, 0x2e, 0x31,
			0x33, 0x4e, 0x2f, 0x30, 0x31,
			0x33, 0x31, 0x39, 0x2e, 0x39,
			0x38, 0x45, 0x2d, 0x30, 0x32,
			0x37, 0x2f, 0x30, 0x30, 0x30,
			0x2f, 0x44, 0x65, 0x6e, 0x69,
			0x73, 0x20, 0x7a, 0x75, 0x20,
			0x48, 0x61, 0x75, 0x73, 0x65,
			0x0d, 0x61, 0x31 // $$CRC3161
    };

    //                                  1         2         3         4         5         6         7         8         9
    //                        0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0....5....0
    char dstarCRCGPS_2[98] = "$$CRC8C55,ALBERTO-9>API510,DSTAR*:/182454h4318.59N/00638.13E>/A=000000ICOM-ID5100 TX 10w ant.97cm";
    dstarCRCGPS_2[97] = 0x0d;

    unsigned int dstarCRCGPS_2_crc = 0x8c55;

    std::cout << std::endl;

    DSDcc::DStarCRC dStarCRC;

    if (dStarCRC.check_crc(dstarHeader, 41)) {
    	std::cout << "Test DStar CRC OK" << std::endl;
    } else {
    	std::cout << "Test DStar CRC KO" << std::endl;
    }

    DSDcc::CRC dstarCRC2(DSDcc::CRC::PolyDStar16, 16, 0xffff, 0xffff, 1, 0, 0);

    unsigned long crc2 = crc.crctablefast((unsigned char *)dstarHeader, 39UL);
    unsigned long crc_decoded = (dstarHeader[40] << 8) + dstarHeader[39]; //inversion msb lsb

    if (crc2 == crc_decoded) {
    	std::cout << "Test DStar 2 CRC OK" << std::endl;
    } else {
    	std::cout << "Test DStar 2 CRC KO" << std::endl;
    }

    if (dStarCRC.check_crc(dstarCRCGPS, 71+2)) {
    	std::cout << "Test DStar $$CRC OK" << std::endl;
    } else {
    	std::cout << "Test DStar $$CRC KO" << std::endl;
    }

    if (dStarCRC.check_crc(dstarCRCGPS, 71, 0x3161)) {
        std::cout << "Test DStar $$CRC with crc OK" << std::endl;
    } else {
        std::cout << "Test DStar $$CRC with crc KO" << std::endl;
    }

    crc2 = crc.crctablefast((unsigned char *)dstarCRCGPS, 71);
    crc_decoded = (dstarCRCGPS[71+1] << 8) + dstarCRCGPS[71]; //inversion msb lsb

    if (crc2 == crc_decoded) {
    	std::cout << "Test DStar 2 $$CRC OK" << std::endl;
    } else {
    	std::cout << "Test DStar 2 $$CRC KO" << std::endl;
    }

    if (dStarCRC.check_crc((unsigned char *) &dstarCRCGPS_2[10], ((int) strlen(dstarCRCGPS_2) - 11), dstarCRCGPS_2_crc)) {
        std::cout << "Test DStar $$CRC_2 OK" << std::endl;
    } else {
        std::cout << "Test DStar $$CRC_2 KO" << std::endl;
    }

    std::cout << ((int) strlen(dstarCRCGPS_2) - 11) << std::endl;

    testNXDN();

    return 0;
}
