/*
 *   Copyright (C) 2015,2016,2018 by Jonathan Naylor G4KLX
 *
 *   Copyright (C) 2018 by Edouard Griffiths F4EXB:
 *   - Cosmetic changes to integrate with DSDcc
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#if !defined(NXDNConvolution_H)
#define  NXDNConvolution_H

#include <stdint.h>

namespace DSDcc
{

class CNXDNConvolution {
public:
	CNXDNConvolution();
	~CNXDNConvolution();

	void start();
	void decode(uint8_t s0, uint8_t s1);
	void chainback(unsigned char* out, unsigned int nBits);

	void encode(const unsigned char* in, unsigned char* out, unsigned int nBits) const;

private:
	uint16_t* m_metrics1;
	uint16_t* m_metrics2;
	uint16_t* m_oldMetrics;
	uint16_t* m_newMetrics;
	uint64_t* m_decisions;
	uint64_t* m_dp;
	static const uint8_t BIT_MASK_TABLE[];
	static const uint8_t BRANCH_TABLE1[];
	static const uint8_t BRANCH_TABLE2[];
	static const unsigned int NUM_OF_STATES_D2;
	static const unsigned int NUM_OF_STATES;
	static const uint32_t M;
	static const unsigned int K;
};

} // namespace

#endif

