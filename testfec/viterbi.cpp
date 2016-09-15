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
#include <stdio.h>
#include <sys/time.h>

#include "../viterbi.h"
#include "../descramble.h"

long long getUSecs()
{
    struct timeval tp;
    gettimeofday(&tp, 0);
    return (long long) tp.tv_sec * 1000000L + tp.tv_usec;
}

void printByteArray(const unsigned char *byteArray, int nbBytes)
{
	for (int i = 0; i < nbBytes; i++)
	{
		std::cout << (int) byteArray[i] << " ";
	}

	std::cout << std::endl;
}

void bitify(unsigned char *bitArray, const char *chars, int nbChars)
{
	for (int is = 0; is < nbChars; is++)
	{
		for (int ib = 0; ib < 8; ib++)
		{
			bitArray[8*is+ib] = (chars[is]>>(7-ib)) & 1;
		}
	}
}

void charify(char *chars, const unsigned char *bitArray, int nbBits)
{
	char c;
	int ic = 0;

	for (int ib = 0; ib < nbBits; ib++)
	{
		if ((ib%8) == 0)
		{
			c = 0;
		}

		int biti = 7 - (ib%8);
		c += bitArray[ib] << biti;

		if ((ib%8) == 7)
		{
			chars[ic] = c;
			ic++;
		}
	}
}

void testBitifyCharify()
{
	char text[44];
	unsigned char bits[43*8];
	char textBack[44];

	sprintf(text, "The quick brown fox jumps over the lazy dog");
	bitify(bits, text, 43);
	charify(textBack, bits, 43*8);
	textBack[43] = 0;

	std::cout << "Test bitify/charify" << std::endl;
	std::cout << "-------------------" << std::endl;
	std::cout << textBack << std::endl << std::endl;
}

void testMIT()
{
	const unsigned char mitCodewords[] = {0, 3, 2, 1, 3, 0, 1, 2};

	const unsigned char validPredA[] = {0, 2, 0, 2};
	const unsigned char validPredB[] = {1, 3, 1, 3};
	const unsigned char validBitA[]  = {0, 0, 1, 1};
	const unsigned char validBitB[]  = {0, 0, 1, 1};

	const unsigned char dataBitsA[6] = {1, 0, 1, 1, 0, 0};
	const unsigned char correctSymbolsA[6] = {3, 3, 1, 0, 1, 2};
	unsigned char symbolsA[6];

	std::cout << "Test (MIT) K=3 N=2 Polys={1+x+x^2, 1+x}" << std::endl;
	std::cout << "---------------------------------------" << std::endl;

	DSDcc::Viterbi viterbi23(3, 2, DSDcc::Viterbi::Poly23);
	const unsigned char *codes = viterbi23.getBranchCodes();

	if (memcmp(codes, mitCodewords, 8) == 0)
	{
		std::cout << "Branch codewords are valid" << std::endl;
	}
	else
	{
		std::cout << "Branch codewords are invalid: " << std::endl;
		printByteArray(codes, 8);
	}

	const unsigned char *predA = viterbi23.getPredA();
	const unsigned char *predB = viterbi23.getPredB();
	const unsigned char *bitA  = viterbi23.getBitA();
	const unsigned char *bitB  = viterbi23.getBitB();
	bool predOK = true;

	if (memcmp(validPredA, predA, 4) == 0) {
	    std::cout << "First set of predecessor valid" << std::endl;
	} else {
	    predOK = false;
	}

    if (memcmp(validPredB, predB, 4) == 0) {
        std::cout << "Second set of predecessor valid" << std::endl;
    } else {
        predOK = false;
    }

    if (memcmp(validBitA, bitA, 4) == 0) {
        std::cout << "First set of predecessor bit transitions valid" << std::endl;
    } else {
        predOK = false;
    }

    if (memcmp(validBitA, bitB, 4) == 0) {
        std::cout << "Second set of predecessor bit transitions valid" << std::endl;
    } else {
        predOK = false;
    }

    if (!predOK)
    {
        for (int s = 0; s < 4; s++)
        {
            std::cout << "S" << s << ": " << (int) predA[s] << " (" << (int) bitA[s] << ")" << std::endl;
            std::cout << " " << s << ": " << (int) predB[s] << " (" << (int) bitB[s] << ")" << std::endl;
        }
    }

	std::cout << "-- Test #1 --" << std::endl;

	long long ts = getUSecs();
	viterbi23.encodeToSymbols(symbolsA, dataBitsA, 6, 0);
	long long usecs = getUSecs() - ts;
	std::cout << "A encoded: in " << usecs << " microseconds" << std::endl;

	if (memcmp(symbolsA, correctSymbolsA, 6) == 0)
	{
		std::cout << "A encoded: codewords are valid" << std::endl;
	}
	else
	{
		std::cout << "A endoded: codewords are invalid: " << std::endl;
		printByteArray(symbolsA, 6);
	}

	unsigned char decodedDataBitsA[6];

	ts = getUSecs();
	viterbi23.decodeFromSymbols(decodedDataBitsA, symbolsA, 6, 0);
	usecs = getUSecs() - ts;
	std::cout << "A decoded: in " << usecs << " microseconds" << std::endl;

    if (memcmp(decodedDataBitsA, dataBitsA, 6) == 0)
    {
        std::cout << "A decoded: bits are valid" << std::endl;
    }
    else
    {
        std::cout << "A decoded: bits are invalid: " << std::endl;
        printByteArray(decodedDataBitsA, 6);
    }
                                           //{0, 1, 2, 0, 0, 0};
    const unsigned char corruptSymbolsA[6] = {3, 2, 3, 0, 1, 2};

	ts = getUSecs();
	viterbi23.decodeFromSymbols(decodedDataBitsA, corruptSymbolsA, 6, 0);
	usecs = getUSecs() - ts;
	std::cout << "~A decoded: in " << usecs << " microseconds" << std::endl;

    if (memcmp(decodedDataBitsA, dataBitsA, 6) == 0)
    {
        std::cout << "~A decoded: bits are valid" << std::endl;
    }
    else
    {
        std::cout << "~A decoded: bits are invalid: " << std::endl;
        printByteArray(decodedDataBitsA, 6);
    }

    char text[44], decodedText[44];
    sprintf(text, "The quick brown fox jumps over the lazy dog");
    unsigned char bitsPh[43*8];
    unsigned char symbolsPh[43*8];
    unsigned char decodedBitsPh[43*8];

    bitify(bitsPh, text, 43);
	ts = getUSecs();
    viterbi23.encodeToSymbols(symbolsPh, bitsPh, 43*8, 0);
	usecs = getUSecs() - ts;
	std::cout << "-- Test #2 --" << std::endl;
	std::cout << "Phrase encoded: in " << usecs << " microseconds" << std::endl;

	ts = getUSecs();
	viterbi23.decodeFromSymbols(decodedBitsPh, symbolsPh, 43*8, 0);
	usecs = getUSecs() - ts;
	std::cout << "Phrase decoded: in " << usecs << " microseconds" << std::endl;

	charify(decodedText, decodedBitsPh, 43*8);
	decodedText[43] = '\0';

	std::cout << "Phrase: " << decodedText << std::endl;
	std::cout << std::endl;
}

void testViterbiLegacy()
{
    char text[41], decodedText[41];
    sprintf(text, "The quick brown fox jumps over the lazy    ");
    unsigned char bitsPh[41*8 + 2];
    unsigned char symbolsPh[41*8 + 2];
    unsigned char decodedBitsPh[41*8 + 2];

	std::cout << "Test legacy Viterbi decoder from DSD" << std::endl;
	std::cout << "------------------------------------" << std::endl;
	std::cout << "-- test new Viterbi class --" << std::endl;

	DSDcc::Viterbi viterbi(3, 2, DSDcc::Viterbi::Poly23a, false); // false = dibit coding is LSB first for D-Star

    bitify(bitsPh, text, 41);
    bitsPh[328] = 0;
    bitsPh[329] = 0;
	long long ts = getUSecs();
	viterbi.encodeToSymbols(symbolsPh, bitsPh, 41*8 + 2, 0);
	long long usecs = getUSecs() - ts;
	std::cout << "Phrase encoded: in " << usecs << " microseconds" << std::endl;

	for (int i = 0; i < 41*8 + 2; i++) // flip some bits
	{
	    if (i%6 == 5) {
	        symbolsPh[i] ^= 1;
	    }
	}

	ts = getUSecs();
	viterbi.decodeFromSymbols(decodedBitsPh, symbolsPh, 41*8 + 2, 0);
	usecs = getUSecs() - ts;
	std::cout << "Phrase decoded: in " << usecs << " microseconds" << std::endl;

	charify(decodedText, decodedBitsPh, 41*8);
	decodedText[41] = '\0';

	std::cout << "Phrase: " << decodedText << std::endl;

	std::cout << "-- Legacy DSD Viterbi class --" << std::endl;

	unsigned char symbolsPhBits[660];

	for (int i = 0; i < 660; i++)
	{
	    symbolsPhBits[i] = (symbolsPh[i/2]>>(i%2)) & 1;
	}

    ts = getUSecs();
	DSDcc::Descramble::FECdecoder(symbolsPhBits, decodedBitsPh);
    usecs = getUSecs() - ts;
    std::cout << "Phrase decoded: in " << usecs << " microseconds" << std::endl;

    charify(decodedText, decodedBitsPh, 41*8);
    decodedText[41] = '\0';

    std::cout << "Phrase: " << decodedText << std::endl;
}

void testViterbi(DSDcc::Viterbi& viterbi)
{
	const unsigned char dataBitsA[6] = {1, 0, 1, 1, 0, 0};
	const unsigned char corruptSymbols[6] = {0, 0, 2, 1, 0, 0};
	unsigned char symbolsA[6];

	std::cout << "-- Test #1 --" << std::endl;

	long long ts = getUSecs();
	viterbi.encodeToSymbols(symbolsA, dataBitsA, 6, 0);
	long long usecs = getUSecs() - ts;
	std::cout << "A encoded: in " << usecs << " microseconds" << std::endl;

	unsigned char decodedDataBitsA[6];

	ts = getUSecs();
	viterbi.decodeFromSymbols(decodedDataBitsA, symbolsA, 6, 0);
	usecs = getUSecs() - ts;
	std::cout << "A decoded: in " << usecs << " microseconds" << std::endl;

    if (memcmp(decodedDataBitsA, dataBitsA, 6) == 0)
    {
        std::cout << "A decoded: bits are valid" << std::endl;
    }
    else
    {
        std::cout << "A decoded: bits are invalid: " << std::endl;
        printByteArray(decodedDataBitsA, 6);
    }

    unsigned char corruptSymbolsA[6];

    for (int i = 0; i < 6; i++)
    {
    	corruptSymbolsA[i] = corruptSymbols[i] ^ symbolsA[i];
    }

	ts = getUSecs();
	viterbi.decodeFromSymbols(decodedDataBitsA, corruptSymbolsA, 6, 0);
	usecs = getUSecs() - ts;
	std::cout << "~A decoded: in " << usecs << " microseconds" << std::endl;

    if (memcmp(decodedDataBitsA, dataBitsA, 6) == 0)
    {
        std::cout << "~A decoded: bits are valid" << std::endl;
    }
    else
    {
        std::cout << "~A decoded: bits are invalid: " << std::endl;
        printByteArray(decodedDataBitsA, 6);
    }

    char text[44], decodedText[44];
    sprintf(text, "The quick brown fox jumps over the lazy dog");
    unsigned char bitsPh[43*8];
    unsigned char symbolsPh[43*8];
    unsigned char decodedBitsPh[43*8];

    bitify(bitsPh, text, 43);
	ts = getUSecs();
	viterbi.encodeToSymbols(symbolsPh, bitsPh, 43*8, 0);
	usecs = getUSecs() - ts;
	std::cout << "-- Test #2 --" << std::endl;
	std::cout << "Phrase encoded: in " << usecs << " microseconds" << std::endl;

    for (int i = 0; i < 43*8; i++) // flip some bits
    {
        if (i%6 == 5) {
            symbolsPh[i] ^= 1;
        }
    }

	ts = getUSecs();
	viterbi.decodeFromSymbols(decodedBitsPh, symbolsPh, 43*8, 0);
	usecs = getUSecs() - ts;
	std::cout << "Phrase decoded: in " << usecs << " microseconds" << std::endl;

	charify(decodedText, decodedBitsPh, 43*8);
	decodedText[43] = '\0';

	std::cout << "Phrase: " << decodedText << std::endl;
}

void testDStar()
{
	std::cout << "Test (D-Star) K=3 N=2 Polys={1+x+x^2, 1+x^2}" << std::endl;
	std::cout << "--------------------------------------------" << std::endl;

	DSDcc::Viterbi viterbi23a(3, 2, DSDcc::Viterbi::Poly23a);

	testViterbi(viterbi23a);

	std::cout << std::endl;
}

void test24()
{
	std::cout << "Test K=4 N=2 with Polys={1+x+x^2+x^3, 1+x^2+x^3}" << std::endl;
	std::cout << "------------------------------------------------" << std::endl;

	DSDcc::Viterbi viterbi24(4, 2, DSDcc::Viterbi::Poly24);

	testViterbi(viterbi24);

	std::cout << std::endl;
}

void test25()
{
	std::cout << "Test K=5 N=2 with Polys={1+x^2+x^3+x^4, 1+x+x^4}" << std::endl;
	std::cout << "------------------------------------------------" << std::endl;

	DSDcc::Viterbi viterbi25(5, 2, DSDcc::Viterbi::Poly25);

	testViterbi(viterbi25);

	std::cout << std::endl;
}

void testYSF()
{
	std::cout << "Test (YSF) K=5 N=2 Polys={1+x^3+x^4, 1+x+x^2+X^4}" << std::endl;
	std::cout << "-------------------------------------------------" << std::endl;

	DSDcc::Viterbi viterbi25y(5, 2, DSDcc::Viterbi::Poly25y);

	testViterbi(viterbi25y);

	std::cout << std::endl;
}

int main(int argc, char *argv[])
{
	testBitifyCharify();
	testMIT();
	testDStar();
	test24();
	test25();
	testYSF();
	testViterbiLegacy();
	return 0;
}
