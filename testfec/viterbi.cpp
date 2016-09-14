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
#include "../viterbi.h"

void testMIT()
{
	const unsigned char mitCodewords[] = {0, 3, 2, 1, 3, 0, 1, 2};

	const unsigned char dataBitsA[6] = {1, 0, 1, 1, 0, 0};
	const unsigned char correctSymbolsA[6] = {3, 3, 1, 0, 1, 2};
	unsigned char symbolsA[6];

	std::cout << "Test (MIT) K=3 N=2 Polys={1+x+x^2, 1+x}" << std::endl;

	DSDcc::Viterbi viterbi23(3, 2, DSDcc::Viterbi::Poly23);
	const unsigned char *codes = viterbi23.getBranchCodes();

	if (memcmp(codes, mitCodewords, 8) == 0)
	{
		std::cout << "Branch codewords are valid" << std::endl;
	}
	else
	{
		std::cout << "Branch codewords are invalid: " << std::endl;

		for (int i = 0; i < 8; i++)
		{
			std::cout << (int) codes[i] << " ";
		}

		std::cout << std::endl;
	}

	const unsigned char *predA = viterbi23.getPredA();
	const unsigned char *predB = viterbi23.getPredB();
	const unsigned char *bitA  = viterbi23.getBitA();
	const unsigned char *bitB  = viterbi23.getBitB();

	for (int s = 0; s < 4; s++)
	{
		std::cout << "S" << s << ": " << (int) bitA[s] << ":" << (int) predA[s] << std::endl;
		std::cout << " " << s << ": " << (int) bitB[s] << ":" << (int) predB[s] << std::endl;
	}

	viterbi23.encodeToSymbols(symbolsA, dataBitsA, 6, 0);

	if (memcmp(symbolsA, correctSymbolsA, 6) == 0)
	{
		std::cout << "A encoded: codewords are valid" << std::endl;
	}
	else
	{
		std::cout << "A endoded: codewords are invalid: " << std::endl;

		for (int i = 0; i < 6; i++)
		{
			std::cout << (int) symbolsA[i] << " ";
		}

		std::cout << std::endl;
	}
}

int main(int argc, char *argv[])
{
	testMIT();
	return 0;
}
