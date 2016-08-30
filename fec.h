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

#ifndef FEC_H_
#define FEC_H_

namespace DSDcc
{

class Hamming_7_4
{
public:
	Hamming_7_4();
	~Hamming_7_4();

	void init();
	bool decode(unsigned char *rxBits);

private:
	unsigned char m_corr[8];             //!< single bit error correction by syndrome index
	static const unsigned char m_H[7*3]; //!< Parity check matrix of bits
};

class Hamming_12_8
{
public:
    Hamming_12_8();
    ~Hamming_12_8();

    void init();
    bool decode(unsigned char *rxBits, unsigned char *decodedBits, int nbCodewords);

private:
    unsigned char m_corr[16];                       //!< single bit error correction by syndrome index
    static const unsigned char m_H[12*4]; //!< Parity check matrix of bits
};

class Golay_20_8
{
public:
	Golay_20_8();
	~Golay_20_8();

	void init();
	bool decode(unsigned char *rxBits);

private:
	unsigned char m_corr[4096][3];         //!< up to 3 bit error correction by syndrome index
	static const unsigned char m_H[20*12]; //!< Parity check matrix of bits
};

class QR_16_7_6
{
public:
	QR_16_7_6();
	~QR_16_7_6();

	void init();
	bool decode(unsigned char *rxBits);

private:
	unsigned char m_corr[512][2];          //!< up to 2 bit error correction by syndrome index
	static const unsigned char m_H[16*9];  //!< Parity check matrix of bits
};

} // namespace DSDcc


#endif /* FEC_H_ */
