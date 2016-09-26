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

#include "crc.h"

namespace DSDcc
{

const unsigned long CRC::PolyCCITT16 = 0x1021;

CRC::CRC(unsigned long polynomial,
            int order,
            unsigned long crcinit,
            unsigned long crcxor,
            int direct,
            int refin,
            int refout) :
        m_order(order),
        m_poly(polynomial),
        m_direct(direct),
        m_crcinit(crcinit),
        m_crcxor(crcxor),
        m_refin(refin),
        m_refout(refout)
{
    m_crcmask = ((((unsigned long) 1 << (m_order - 1)) - 1) << 1) | 1;
    m_crchighbit = (unsigned long) 1 << (m_order - 1);

    generate_crc_table();
    init();
}

CRC::~CRC()
{
}

unsigned long CRC::reflect(unsigned long crc, int bitnum)
{
    // reflects the lower 'bitnum' bits of 'crc'

    unsigned long i, j = 1, crcout = 0;

    for (i = (unsigned long) 1 << (bitnum - 1); i; i >>= 1)
    {
        if (crc & i)
            crcout |= j;
        j <<= 1;
    }
    return (crcout);
}

void CRC::generate_crc_table()
{
    // make CRC lookup table used by table algorithms

    int i, j;
    unsigned long bit, crc;

    for (i = 0; i < 256; i++)
    {
        crc = (unsigned long) i;

        if (m_refin)
            crc = reflect(crc, 8);

        crc <<= m_order - 8;

        for (j = 0; j < 8; j++)
        {
            bit = crc & m_crchighbit;
            crc <<= 1;

            if (bit)
                crc ^= m_poly;
        }

        if (m_refin)
            crc = reflect(crc, m_order);
        crc &= m_crcmask;
        m_crctab[i] = crc;
    }
}

void CRC::init()
{
    int i, j;
    unsigned long bit, crc;

    if (!m_direct)
    {
        m_crcinit_nondirect = m_crcinit;
        crc = m_crcinit;

        for (i = 0; i < m_order; i++)
        {
            bit = crc & m_crchighbit;
            crc <<= 1;

            if (bit)
                crc ^= m_poly;
        }

        crc &= m_crcmask;
        m_crcinit_direct = crc;
    }
    else
    {
        m_crcinit_direct = m_crcinit;
        crc = m_crcinit;

        for (i = 0; i < m_order; i++)
        {
            bit = crc & 1;

            if (bit)
                crc ^= m_poly;

            crc >>= 1;

            if (bit)
                crc |= m_crchighbit;
        }

        m_crcinit_nondirect = crc;
    }
}

unsigned long CRC::crctablefast(unsigned char* p, unsigned long len)
{
    // fast lookup table algorithm without augmented zero bytes, e.g. used in pkzip.
    // only usable with polynom orders of 8, 16, 24 or 32.

    unsigned long crc = m_crcinit_direct;

    if (m_refin)
        crc = reflect(crc, m_order);

    if (!m_refin)
        while (len--)
            crc = (crc << 8) ^ m_crctab[((crc >> (m_order - 8)) & 0xff) ^ *p++];
    else
        while (len--)
            crc = (crc >> 8) ^ m_crctab[(crc & 0xff) ^ *p++];

    if (m_refout ^ m_refin)
        crc = reflect(crc, m_order);

    crc ^= m_crcxor;
    crc &= m_crcmask;

    return (crc);
}

unsigned long CRC::crctable(unsigned char* p, unsigned long len)
{
    // normal lookup table algorithm with augmented zero bytes.
    // only usable with polynom orders of 8, 16, 24 or 32.

    unsigned long crc = m_crcinit_nondirect;

    if (m_refin)
        crc = reflect(crc, m_order);

    if (!m_refin)
        while (len--)
            crc = ((crc << 8) | *p++) ^ m_crctab[(crc >> (m_order - 8)) & 0xff];
    else
        while (len--)
            crc = ((crc >> 8) | (*p++ << (m_order - 8))) ^ m_crctab[crc & 0xff];

    if (!m_refin)
        while (++len < m_order / 8)
            crc = (crc << 8) ^ m_crctab[(crc >> (m_order - 8)) & 0xff];
    else
        while (++len < m_order / 8)
            crc = (crc >> 8) ^ m_crctab[crc & 0xff];

    if (m_refout ^ m_refin)
        crc = reflect(crc, m_order);

    crc ^= m_crcxor;
    crc &= m_crcmask;

    return (crc);
}

unsigned long CRC::crcbitbybit(unsigned char* p, unsigned long len)
{
    // bit by bit algorithm with augmented zero bytes.
    // does not use lookup table, suited for polynom orders between 1...32.

    unsigned long i, j, c, bit;
    unsigned long crc = m_crcinit_nondirect;

    for (i = 0; i < len; i++)
    {
        c = (unsigned long) *p++;

        if (m_refin)
            c = reflect(c, 8);

        for (j = 0x80; j; j >>= 1)
        {

            bit = crc & m_crchighbit;
            crc <<= 1;

            if (c & j)
                crc |= 1;

            if (bit)
                crc ^= m_poly;
        }
    }

    for (i = 0; i < m_order; i++)
    {
        bit = crc & m_crchighbit;
        crc <<= 1;

        if (bit)
            crc ^= m_poly;
    }

    if (m_refout)
        crc = reflect(crc, m_order);

    crc ^= m_crcxor;
    crc &= m_crcmask;

    return (crc);
}

unsigned long CRC::crcbitbybitfast(unsigned char* p, unsigned long len)
{
    // fast bit by bit algorithm without augmented zero bytes.
    // does not use lookup table, suited for polynom orders between 1...32.

    unsigned long i, j, c, bit;
    unsigned long crc = m_crcinit_direct;

    for (i = 0; i < len; i++)
    {
        c = (unsigned long) *p++;

        if (m_refin)
            c = reflect(c, 8);

        for (j = 0x80; j; j >>= 1)
        {
            bit = crc & m_crchighbit;
            crc <<= 1;

            if (c & j)
                bit ^= m_crchighbit;

            if (bit)
                crc ^= m_poly;
        }
    }

    if (m_refout)
        crc = reflect(crc, m_order);

    crc ^= m_crcxor;
    crc &= m_crcmask;

    return (crc);
}

// ====================================================================

DStarCRC::DStarCRC() : crc(0)
{}

DStarCRC::~DStarCRC()
{}

void DStarCRC::fcsbit(unsigned char tbyte)
{
	  crc ^= tbyte;

	  if (crc & 1)
	  {
		    crc = (crc >> 1) ^ 0x8408;  // X-modem CRC poly
	  }
	  else
	  {
		    crc = crc >> 1;
	  }
}


void DStarCRC::compute_crc(unsigned char *array, int size_buffer)
{
	crc = 0xffff;

	for (int n = 0; n < (size_buffer - 2); n++)
	{
		for (int m = 0; m < 8; m++)
		{    //each bit must be calculated separatly
			fcsbit(bitRead(array[n], m));
		}
	}

	crc ^= 0xffff;
}


bool DStarCRC::check_crc(unsigned char *array, int size_buffer)
{

	compute_crc(array, size_buffer);

	unsigned int crc_decoded = (array[size_buffer - 1] << 8) + array[size_buffer - 2]; //inversion msb lsb

	if (crc_decoded == crc)
	{
		return true;
	}
	else
	{
		return false;
	}
}

} // namespace DSDcc
